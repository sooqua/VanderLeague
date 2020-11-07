#include "mmap.hpp"

#pragma warning(disable: 4090)
#pragma warning(disable: 4302)
#pragma warning(disable: 4311)
#pragma warning(disable: 4312)

mmap::mmap() {
	proc = kernelmode_proc_handler();
	data_size = 0;
	raw_image_data = NULL;
}

bool mmap::attach_to_process(const char* process_name_arg) {
	this->process_name = process_name_arg;
	if (!proc.attach(process_name_arg)) {
		LOG_ERROR("Unable to attach to process!");
		return false;
	}

	LOG("Attached to process %s successfully...", process_name_arg);
	return true;
}

bool mmap::load_dll(const char* file_name) {
	std::ifstream f(file_name, std::ios::binary | std::ios::ate);

	if (!f) {
		LOG_ERROR("Unable to open DLL file!");
		return false;
	}

	std::ifstream::pos_type pos{ f.tellg() };
	data_size = pos;

	raw_image_data = new UINT8[data_size];

	if (!raw_image_data)
		return false;

	f.seekg(0, std::ios::beg);
	f.read((char*)raw_image_data, data_size);

	f.close();
	return true;
}

DWORD getProcessId(LPCTSTR ProcessName)
{
	PROCESSENTRY32 pt;
	HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pt.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hsnap, &pt)) {
		do {
			if (!lstrcmpi(pt.szExeFile, ProcessName)) {
				CloseHandle(hsnap);
				return pt.th32ProcessID;
			}
		} while (Process32Next(hsnap, &pt));
	}
	CloseHandle(hsnap);
	return 0;
}

void suspend(DWORD processId)
{
	HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	THREADENTRY32 threadEntry;
	threadEntry.dwSize = sizeof(THREADENTRY32);

	Thread32First(hThreadSnapshot, &threadEntry);

	do
	{
		if (threadEntry.th32OwnerProcessID == processId)
		{
			HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE,
				threadEntry.th32ThreadID);

			if (hThread) {
				SuspendThread(hThread);
				CloseHandle(hThread);
			}
		}
	} while (Thread32Next(hThreadSnapshot, &threadEntry));

	CloseHandle(hThreadSnapshot);
}

bool mmap::inject() {

	if (!proc.is_attached()) {
		LOG_ERROR("Not attached to process!");
		return false;
	}

	if (!raw_image_data) {
		LOG_ERROR("Data buffer is empty!");
		return false;
	}

	// TODO: Call original GetBoundingRadius()
	UINT8 hook[] = {
		0x9C,                       // pushfd
		0x50,                       // push eax
		0x51,                       // push ecx
		0x52,                       // push edx
		0x55,                       // push ebp
		0x56,                       // push esi
		0x53,                       // push ebx
		0x57,                       // push edi
		0x6A, 0x00,                    // push 0
		0x6A, 0x01,                    // push 1
		0x68, 0x11, 0x11, 0x11, 0x11,              // push 11111111
		0xB8, 0x11, 0x11, 0x11, 0x11,              // mov eax,11111111
		0xFF, 0xD0,                     // call eax
		0x5F,                       // pop edi
		0x5B,                       // pop ebx
		0x5E,                       // pop esi
		0x5D,                       // pop ebp
		0x5A,                       // pop edx
		0x59,                       // pop ecx
		0x58,                       // pop eax
		0x9D,                       // popfd
		0x68, 0x11, 0x11, 0x11, 0x11,              // push 11111111
		0xC3                       // ret
	};

	IMAGE_DOS_HEADER* image_dos_header{ (IMAGE_DOS_HEADER*)raw_image_data };

	if (image_dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
		LOG_ERROR("Invalid image DOS header signature!");
		return false;
	}

	IMAGE_NT_HEADERS32* image_nt_header{ (IMAGE_NT_HEADERS32*)(&raw_image_data[image_dos_header->e_lfanew]) };

	if (image_nt_header->Signature != IMAGE_NT_SIGNATURE) {
		LOG_ERROR("Invalid image NT header signature!");
		return false;
	}

	/*auto d3d9base = proc.get_table_module_base();
	if (!d3d9base) {
		LOG_ERROR("Cannot find d3d9.dll base");
		return false;
	}

	LOG("Found d3d9.dll base: %llx", d3d9base);*/

	UINT32 image_base = static_cast<UINT32>(proc.virtual_alloc(
		static_cast<UINT64>(image_nt_header->OptionalHeader.SizeOfImage),
		(UINT64)(MEM_COMMIT | MEM_RESERVE),
		(UINT64)(PAGE_EXECUTE_READWRITE)));

	if (!image_base) {
		LOG_ERROR("Unable to allocate memory for the image!");
		return false;
	}

	LOG("Image base: 0x%p", reinterpret_cast<void*>(image_base));

	UINT64 stub_base = proc.virtual_alloc(
		sizeof(hook),
		MEM_COMMIT | MEM_RESERVE,
		PAGE_EXECUTE_READWRITE);

	if (!stub_base) {
		LOG_ERROR("Unable to allocate memory for the stub!");
		return false;
	}

	LOG("Stub base: 0x%p", reinterpret_cast<void*>(stub_base));

	PIMAGE_IMPORT_DESCRIPTOR import_descriptor{ reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(get_ptr_from_rva(
		(UINT32)(image_nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress),
		image_nt_header,
		raw_image_data)) };

	if (image_nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) {
		LOG("Solving imports...");
		solve_imports(raw_image_data, image_nt_header, import_descriptor);
	}

	PIMAGE_BASE_RELOCATION base_relocation{ (PIMAGE_BASE_RELOCATION)get_ptr_from_rva(
			image_nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress,
			image_nt_header,
			raw_image_data) };

	if (image_nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) {
		LOG("Solving relocations...");
		solve_relocations(
			reinterpret_cast<UINT32>(raw_image_data),
			image_base,
			image_nt_header,
			base_relocation,
			image_nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size);
	}

	if (!parse_imports()) {
		LOG_ERROR("Unable to parse imports!");
		return false;
	}

	LOG("Parse imports ok...");

	/*auto read_ptr = [this](UINT64 base, std::vector<UINT64> offsets) -> UINT64 {
		auto ret_addr = base;
		for (auto off : offsets) {
			ret_addr = read_memory<UINT32>(ret_addr + off, sizeof(UINT32));
			if (!ret_addr)
				return (UINT64)nullptr;
		}
		return ret_addr;
	};*/

	UINT32 vmt_function_ptr = static_cast<UINT32>(proc.get_table_module_base() + (UINT64)0x10E0);
	if (!vmt_function_ptr) {
		LOG_ERROR("Cannot find vmt table");
		return false;
	}

	vmt_function_ptr += 17 * sizeof(UINT32);
	LOG("VMT function pointer: %#010x", vmt_function_ptr);

	UINT32 orginal_function_addr{ read_memory<UINT32>(vmt_function_ptr) };

	//*(UINT32*)(hook + 0x7) = vmt_function_ptr;
	*(UINT32*)(hook + 0x21) = orginal_function_addr;

	proc.write_memory(
		image_base,
		(UINT64)raw_image_data,
		(UINT64)image_nt_header->FileHeader.SizeOfOptionalHeader + sizeof(image_nt_header->FileHeader) + sizeof(image_nt_header->Signature));

	LOG("Mapping PE sections...");
	map_pe_sections(image_base, image_nt_header);

	UINT32 entry_point = image_base + image_nt_header->OptionalHeader.AddressOfEntryPoint;

	*(UINT32*)(hook + 0xD) = image_base;
	*(UINT32*)(hook + 0x12) = entry_point;

	LOG("Entry point: %#010x", entry_point);

	proc.write_memory(stub_base, (UINT64)hook, sizeof(hook));
	LOG("stub_base: %llu", stub_base);
	system("pause");

	UINT64 stub_base_ptr_game = proc.virtual_alloc(sizeof(UINT32), (UINT64)(MEM_COMMIT | MEM_RESERVE), (UINT64)(PAGE_READWRITE));
	if (!stub_base_ptr_game) {
		LOG_ERROR("Unable to allocate memory for the stub pointer!");
		return false;
	}
	UINT64 stub_base_ptr_inj = (UINT64)(UINT_PTR)(&stub_base);
	proc.write_memory(stub_base_ptr_game, stub_base_ptr_inj, sizeof(UINT32));
	//proc.virtual_protect(vmt_function_ptr, sizeof(UINT32), PAGE_READWRITE);
	proc.write_protected_memory(vmt_function_ptr, stub_base_ptr_game, sizeof(UINT32));
	//proc.virtual_protect(vmt_function_ptr, sizeof(UINT32), PAGE_READONLY);

	LOG("Injected successfully!");
	system("pause");

	delete[] raw_image_data;
	return true;
}

UINT32* mmap::get_ptr_from_rva(UINT32 rva, PIMAGE_NT_HEADERS32 nt_header, UINT8* image_base) {
	PIMAGE_SECTION_HEADER section_header{ get_enclosing_section_header(rva, nt_header) };

	if (!section_header)
		return 0;

	UINT32 delta{ (UINT32)(section_header->VirtualAddress - section_header->PointerToRawData) };
	return (UINT32*)(image_base + rva - delta);
}

PIMAGE_SECTION_HEADER mmap::get_enclosing_section_header(UINT32 rva, PIMAGE_NT_HEADERS32 nt_header) {
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt_header);

	for (unsigned int i = 0; i < nt_header->FileHeader.NumberOfSections; i++, section ++) {
		UINT32 size = section->Misc.VirtualSize;
		if (!size)
			size = section->SizeOfRawData;

		// Is the RVA within this section?
		if ((rva >= section->VirtualAddress) &&
			(rva < (section->VirtualAddress + size)))
			return section;
	}

	return 0;
}

void mmap::solve_imports(UINT8* base, IMAGE_NT_HEADERS32* nt_header, IMAGE_IMPORT_DESCRIPTOR* import_descriptor) {
	char* module;
	while ((module = (char*)get_ptr_from_rva((DWORD)(import_descriptor->Name), nt_header, (PBYTE)base))) {
		//HMODULE local_module{ LoadLibrary(module) };
		/*dll should be compiled statically to avoid loading new libraries
		if (!process.get_module_base(module)) {
				process.load_library(module);
		}*/

		IMAGE_THUNK_DATA32* thunk_data{ (IMAGE_THUNK_DATA32*)get_ptr_from_rva((DWORD)(import_descriptor->FirstThunk), nt_header, (PBYTE)base) };

		while (thunk_data->u1.AddressOfData) {
			IMAGE_IMPORT_BY_NAME* iibn{ (IMAGE_IMPORT_BY_NAME*)get_ptr_from_rva((DWORD)((thunk_data->u1.AddressOfData)), nt_header, (PBYTE)base) };
			thunk_data->u1.Function = (UINT32)(get_proc_address(module, (char*)iibn->Name));
			thunk_data++;
		}
		import_descriptor++;
	}

	return;
}

void mmap::solve_relocations(UINT32 base, UINT32 relocation_base, IMAGE_NT_HEADERS32* nt_header, IMAGE_BASE_RELOCATION* reloc, size_t size) {
	UINT32 image_base{ nt_header->OptionalHeader.ImageBase };
	UINT32 delta{ relocation_base - image_base };
	unsigned int bytes{ 0 };

	while (bytes < size) {
		UINT32* reloc_base{ (UINT32*)get_ptr_from_rva((UINT32)(reloc->VirtualAddress), nt_header, (PBYTE)base) };
		auto num_of_relocations{ (reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD) };
		auto reloc_data = (uint16_t*)((UINT32)reloc + sizeof(IMAGE_BASE_RELOCATION));

		for (unsigned int i = 0; i < num_of_relocations; i++) {
			if ((*reloc_data >> 12) & IMAGE_REL_BASED_HIGHLOW) {
				*reinterpret_cast<UINT32*>(reinterpret_cast<UINT64>(reloc_base) + (*reloc_data & 0x0FFF)) += delta;
			}
			reloc_data++;
		}

		bytes += reloc->SizeOfBlock;
		reloc = (IMAGE_BASE_RELOCATION*)reloc_data;
	}

	return;
}

void mmap::map_pe_sections(UINT32 base, IMAGE_NT_HEADERS32* nt_header) {
	auto header{ IMAGE_FIRST_SECTION(nt_header) };
	size_t virtual_size{ 0 };
	size_t bytes{ 0 };

	while (nt_header->FileHeader.NumberOfSections && (bytes < nt_header->OptionalHeader.SizeOfImage)) {
		proc.write_memory(static_cast<UINT64>(base) + header->VirtualAddress,
			reinterpret_cast<UINT64>(raw_image_data + header->PointerToRawData), header->SizeOfRawData);
		virtual_size = header->VirtualAddress;
		virtual_size = (++header)->VirtualAddress - virtual_size;
		bytes += virtual_size;

		/*
			TODO:
			Add page protection
		*/
	}

	return;
}

UINT32 mmap::get_proc_address(const char* module_name, const char* func) {
	UINT32 remote_module{ static_cast<UINT32>(proc.get_module_base(module_name)) };
	UINT32 local_module{ reinterpret_cast<UINT32>(GetModuleHandle(module_name)) };
	UINT32 delta{ remote_module - local_module };
	UINT32 proc_address = NULL;
	if (local_module != NULL)
		proc_address = reinterpret_cast<UINT32>(GetProcAddress(reinterpret_cast<HMODULE>(local_module), func));
	return static_cast<UINT32>(proc_address + delta);
}

bool mmap::parse_imports() {
	auto base{ proc.get_module_base(process_name.c_str()) };
	if (!base) {
		LOG_ERROR("Cannot get module base");
		return false;
	}

	auto dos_header{ read_memory< IMAGE_DOS_HEADER >(base) };
	auto nt_headers{ read_memory< IMAGE_NT_HEADERS32 >(base + dos_header.e_lfanew) };
	auto descriptor{ read_memory< IMAGE_IMPORT_DESCRIPTOR >(base + nt_headers.OptionalHeader.DataDirectory[1].VirtualAddress) };

	int descriptor_count{ 0 };
	int thunk_count{ 0 };

	while (descriptor.Name) {
		auto first_thunk{ read_memory< IMAGE_THUNK_DATA32 >(base + descriptor.FirstThunk) };
		auto original_first_thunk{ read_memory< IMAGE_THUNK_DATA32 >(base + descriptor.OriginalFirstThunk) };
		thunk_count = 0;

		while (original_first_thunk.u1.AddressOfData) {
			char name[256];
			proc.read_memory(base + original_first_thunk.u1.AddressOfData + 0x2, reinterpret_cast<uintptr_t>(name), 256);
			name[255] = '\0';
			std::string str_name(name);
			auto thunk_offset{ thunk_count * sizeof(UINT32) };

			if (str_name.length() > 0)
				imports[str_name] = static_cast<UINT32>(base + descriptor.FirstThunk + thunk_offset);


			++thunk_count;
			first_thunk = read_memory< IMAGE_THUNK_DATA32 >(base + descriptor.FirstThunk + sizeof(IMAGE_THUNK_DATA32) * thunk_count);
			original_first_thunk = read_memory< IMAGE_THUNK_DATA32 >(base + descriptor.OriginalFirstThunk + sizeof(IMAGE_THUNK_DATA32) * thunk_count);
		}

		++descriptor_count;
		descriptor = read_memory< IMAGE_IMPORT_DESCRIPTOR >(base + nt_headers.OptionalHeader.DataDirectory[1].VirtualAddress + sizeof(IMAGE_IMPORT_DESCRIPTOR) * descriptor_count);
	}

	return (imports.size() > 0);
}
