#pragma once
#include "kernelmode_proc_handler.hpp"
#include <thread>
#include <chrono>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include "logger.hpp"

class mmap {
	kernelmode_proc_handler proc;
	std::string process_name;
	std::map<std::string, uint32_t> imports;
	uint8_t *raw_image_data;
	size_t data_size;
	
public:
	bool attach_to_process(const char* process_name);
	bool load_dll(const char* file_name);
	bool inject();

	mmap();

private:
	//https://github.com/martell/pedump/blob/master/common.h
	uint32_t* get_ptr_from_rva(uint32_t rva, PIMAGE_NT_HEADERS32 nt_header, uint8_t* imageBase);
	PIMAGE_SECTION_HEADER get_enclosing_section_header(uint32_t rva, PIMAGE_NT_HEADERS32 nt_header);

	void solve_imports(uint8_t *base, IMAGE_NT_HEADERS32 *nt_header, IMAGE_IMPORT_DESCRIPTOR *impDesc);
	void solve_relocations(uint32_t base, uint32_t relocation_base, IMAGE_NT_HEADERS32 *nt_header, IMAGE_BASE_RELOCATION *reloc, size_t size);
	void map_pe_sections(uint32_t base, IMAGE_NT_HEADERS32 *nt_header);


	uint32_t get_proc_address(const char* module_name, const char* func);
	bool parse_imports();

	template <typename type>
	type read_memory(UINT64 src, UINT64 size = sizeof(type)) {
		type ret;
		proc.read_memory(src, (UINT64)&ret, size);
		return ret;
	}

};


