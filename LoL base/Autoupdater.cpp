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
			"8B 0D ? ? ? ? 85 C9 74 07 8B 01 6A 01 FF 50 08 8B"
		},
		true,
		true,
		0,
		&offsets::global::oNetClient
	},
	{
		{
			"8B 0D ? ? ? ? 6A 00 50 E8 ? ? ? ? 33"
		},
		true,
		true,
		0,
		&offsets::global::oChatClientPtr
	},
	{
		{
			"8B 0D ? ? ? ? 89 74 24 14"
		},
		true,
		true,
		0,
		&offsets::global::oObjManager
	},
	{
		{
			"A3 ? ? ? ? 83 FA 10 72 32"
		},
		true,
		true,
		0,
		&offsets::global::oZoomClass
	},
	{
		{
			"A1 ? ? ? ? 83 78 08 02 0F 94 C0"
		},
		true,
		true,
		0,
		&offsets::global::oGameInfo
	},
	{
		{
			"8B 0D ? ? ? ? 6A 00 8B 49 34 E8 ? ? ? ? B0"
		},
		true,
		true,
		0,
		&offsets::global::oHudInstance
	},
	{
		{
			"8B 15 ? ? ? ? 83 EC 08 F3"
		},
		true,
		true,
		0,
		&offsets::global::oRenderer
	},
	{
		{
			"8B 0D ? ? ? ? 89 0D"
		},
		true,
		true,
		0,
		&offsets::global::oUnderMouseObject
	},
	{
		{
			"A1 ? ? ? ? 89 54 24 18"
		},
		true,
		true,
		0,
		&offsets::global::oD3DRenderer
	},
	{
		{
			"83 EC 10 56 E8 ? ? ? ? 8B 08"
		},
		true,
		false,
		0,
		&offsets::global::oWorldToScreen
	},
	{
		{
			"A1 ? ? ? ? 85 C0 74 07 05 ? ? ? ? EB 02 33 C0 56"
		},
		true,
		true,
		0,
		&offsets::global::oLocalPlayer
	},
	{
		{
			"F3 0F 11 05 ? ? ? ? 8B 49"
		},
		true,
		true,
		0,
		&offsets::global::oGameTime
	},
	{
		{
			"8B 41 14 8B 51 18"
		},
		true,
		false,
		0,
		&offsets::global::oGetFirstObject
	},
	{
		{
			"E8 ? ? ? ? 8B F0 85 F6 75 E4"
		},
		true,
		false,
		0,
		&offsets::global::oGetNextObject
	},
	{
		{
			"8B 44 24 04 BA"
		},
		true,
		false,
		0,
		&offsets::global::oGameVersion
	},
	{
		{
			"8B 0D ? ? ? ? 6A 00 E8 ? ? ? ? C7"
		},
		true,
		true,
		0,
		&offsets::global::oMenuGUI
	},
	{   // FUNCTIONS
		{
			"83 EC 38 56 8B 74 24 40"
		},
		true,
		false,
		0,
		&offsets::functions::oCastSpell
	},
	{
		{
			"E8 ? ? ? ? 83 C4 1C 80 7F"
		},
		true,
		false,
		0,
		&offsets::functions::oDrawCircle
	},
	{
		{
			"53 8B D9 B8 ? ? ? ? 8B 93"
		},
		true,
		false,
		0,
		&offsets::functions::oGetBasicAttack
	},
	{
		{
			"83 EC 0C 53 8B 5C 24 14 8B CB 56 57 8B 03 FF 90"
		},
		true,
		false,
		0,
		&offsets::functions::oGetAttackCastDelay
	},
	{
		{
			"E8 ? ? ? ? 8B 44 24 1C 83 C4 0C 8B CE"
		},
		true,
		false,
		0,
		&offsets::functions::oGetAttackDelay
	},
	{
		{
			"E8 ? ? ? ? 8B 4F 2C 85 C9 0F"
		},
		true,
		false,
		0,
		&offsets::functions::oGetPing
	},
	{
		{
			"8B 0D ? ? ? ? 85 C9 74 07 8B 01 6A 01 FF 50 08 8B"
		},
		true,
		true,
		0,
		&offsets::functions::oPingInstance
	},
	{
		{
			"E8 ? ? ? ? 8B F8 8B CB 89"
		},
		true,
		false,
		0,
		&offsets::functions::oGetSpellState
	},
	{
		{
			"56 8B F1 E8 ? ? ? ? 84 C0 74 2E"
		},
		true,
		false,
		0,
		&offsets::functions::oIsTargetable
	},
	{
		{
			"56 8B F1 8B 06 8B 80 ? ? ? ? FF D0 84 C0 74 19"
		},
		true,
		false,
		0,
		&offsets::functions::oIsAlive
	},
	{
		{
			"E8 ? ? ? ? 84 C0 74 0A BB"
		},
		true,
		false,
		0,
		&offsets::functions::oIsBaron
	},
	{
		{
			"E8 ? ? ? ? 83 C4 04 84 C0 74 09 5F"
		},
		true,
		false,
		0,
		&offsets::functions::oIsTurret
	},
	{
		{
			"E8 ? ? ? ? 83 C4 04 84 C0 74 38"
		},
		true,
		false,
		0,
		&offsets::functions::oIsInhib
	},
	{
		{
			"E8 ? ? ? ? 83 C4 04 84 C0 74 2B 57"
		},
		true,
		false,
		0,
		&offsets::functions::oIsHero
	},
	{
		{
			"E8 ? ? ? ? 83 C4 04 84 C0 74 16 8B 0F"
		},
		true,
		false,
		0,
		&offsets::functions::oIsMinion
	},
	{
		{
			"E8 ? ? ? ? 33 DB 84 C0 0F 95 C3 8D 1C 5D ? ? ? ? E9"
		},
		true,
		false,
		0,
		&offsets::functions::oIsDragon
	},
	{
		{
			"E8 ? ? ? ? 83 C4 04 84 C0 74 60"
		},
		true,
		false,
		0,
		&offsets::functions::oIsMissile
	},
	{
		{
			"E8 ? ? ? ? 83 C4 04 84 C0 0F 95 C0 C3 CC 8A"
		},
		true,
		false,
		0,
		&offsets::functions::oIsNexus
	},
	{
		{
			"E8 ? ? ? ? 33 C9 83 C4 0C 84"
		},
		true,
		false,
		0,
		&offsets::functions::oIsNotWall
	},
	{
		{
			"E8 ? ? ? ? 8B E8 83 C4 04 85 ED 0F 84 ? ? ? ? 6A 08"
		},
		true,
		false,
		4,
		&offsets::functions::oIsTroy
	},
	{
		{
			"81 EC ? ? ? ? 56 57 8B F9 C7"
		},
		true,
		false,
		0,
		&offsets::functions::oIssueOrder
	},
	{
		{
			"E8 ? ? ? ? 33 C0 5F C2"
		},
		true,
		false,
		0,
		&offsets::functions::oPrintChat
	},
	{
		{
			"A1 ? ? ? ? 56 6A FF"
		},
		true,
		false,
		0,
		&offsets::functions::oSendChat
	}/*,
	{
		{
			"E8 ? ? ? ? 51 D9 1C 24 E8 ? ? ? ? 8B"
		},
		true,
		false,
		0,
		&offsets::functions::oGetTimerExpiry
	}*/
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

			for (auto& pattern : sig.sigs) {
				auto address = find_signature(nullptr, pattern.c_str());

				if (!address) {
					debug::flog("Signature failed: %s\n", pattern.c_str());
					continue;
				}

				if (sig.read)
					address = *reinterpret_cast<uint8_t**>(address + (pattern.find_first_of("?") / 3));
				else if (address[0] == 0xE8) {
					address = address + *reinterpret_cast<uint32_t*>(address + 1) + 5;
				}

				address += sig.additional;

				// Indivisual search hacks
				if (sig.offset == &offsets::functions::oIsTroy) {
					address = address + *reinterpret_cast<uint32_t*>(address + 1) + 5;
				}

				if (sig.sub_base)
					address -= base;


				*sig.offset = reinterpret_cast<uint32_t>(address);
				break;
			}

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