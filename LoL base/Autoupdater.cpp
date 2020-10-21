#include "Autoupdater.h"
#include "Offsets.h"

#include "Debug.h"

#include <Windows.h>
#include <vector>
#include <cinttypes>
#include <string>
#include <thread>
#include <chrono>

uint8_t* Autoupdater::find_signature(const wchar_t* szModule, const char* szSignature) {
	auto module = GetModuleHandle(szModule);
	static auto pattern_to_byte = [](const char* pattern) {
		auto bytes = std::vector<int>{};
		auto start = const_cast<char*>(pattern);
		auto end = const_cast<char*>(pattern) + strlen(pattern);

		for (auto current = start; current < end; ++current) {
			if (*current == '?') {
				++current;
				if (*current == '?')
					++current;
				bytes.push_back(-1);
			}
			else {
				bytes.push_back(strtoul(current, &current, 16));
			}
		}
		return bytes;
	};

	auto dosHeader = (PIMAGE_DOS_HEADER)module;
	auto ntHeaders = (PIMAGE_NT_HEADERS)((uint8_t*)module + dosHeader->e_lfanew);
	auto textSection = IMAGE_FIRST_SECTION(ntHeaders);

	auto sizeOfImage = textSection->SizeOfRawData;
	auto patternBytes = pattern_to_byte(szSignature);
	auto scanBytes = reinterpret_cast<uint8_t*>(module) + textSection->VirtualAddress;

	auto s = patternBytes.size();
	auto d = patternBytes.data();

	auto mbi = MEMORY_BASIC_INFORMATION{ 0 };
	uint8_t* next_check_address = 0;

	for (auto i = 0ul; i < sizeOfImage - s; ++i) {
		bool found = true;
		for (auto j = 0ul; j < s; ++j) {
			auto current_address = scanBytes + i + j;
			if (current_address >= next_check_address) {
				if (!VirtualQuery(reinterpret_cast<void*>(current_address), &mbi, sizeof(mbi)))
					break;

				if (mbi.Protect == PAGE_NOACCESS) {
					i += ((std::uintptr_t(mbi.BaseAddress) + mbi.RegionSize) - (std::uintptr_t(scanBytes) + i));
					i--;
					found = false;
					break;
				}
				else {
					next_check_address = reinterpret_cast<uint8_t*>(mbi.BaseAddress) + mbi.RegionSize;
				}
			}

			if (scanBytes[i + j] != d[j] && d[j] != -1) {
				found = false;
				break;
			}
		}
		if (found) {
			return &scanBytes[i];
		}
	}
	return nullptr;
}

class offset_signature {
public:
	std::vector<std::string> sigs;
	bool sub_base;
	bool read;
	int32_t additional;
	uint32_t* offset;
};

std::vector<offset_signature> sigs = {
	{
		{
			"A1 ? ? ? ? 85 C0 74 07 05 ? ? ? ? EB 02 33 C0 56"
		},
		true,
		true,
		0,
		&offsets::global::oLocalPlayer
	}
};

void Autoupdater::Start() {
	auto base = std::uintptr_t(GetModuleHandle(nullptr));

	//Invalid all
	for (auto& sig : sigs)
		*sig.offset = 0;

	while (true) {
		auto missing_offset = false;
		for (auto& sig : sigs) {

			if (*sig.offset != 0)
				continue;

			uint32_t old = *sig.offset;

			for (auto& pattern : sig.sigs) {
				auto address = find_signature(nullptr, pattern.c_str());

				if (!address) {
					debug::Printf("Signature failed: %s\n", pattern.c_str());
					continue;
				}

				if (sig.read) {
					address = *reinterpret_cast<uint8_t**>(address + (pattern.find_first_of("?") / 3));
				}
				else if (address[0] == 0xE8) {
					address = address + *reinterpret_cast<uint32_t*>(address + 1) + 5;
				}

				if (sig.sub_base)
					address -= base;

				address += sig.additional;

				*sig.offset = reinterpret_cast<uint32_t>(address);
				break;
			}

			debug::Printf("OLD: %#010X, NEW: %#010X", old, *sig.offset);

			if (!*sig.offset) {
				missing_offset = true;
				break;
			}
		}

		if (!missing_offset)
			break;
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(2s);
	}
}