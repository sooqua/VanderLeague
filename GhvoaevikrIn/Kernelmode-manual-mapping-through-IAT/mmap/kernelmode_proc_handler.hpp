#pragma once
#include "logger.hpp"
#include "utils.h"

#include <string>

#define ioctl_read_memory CTL_CODE(FILE_DEVICE_UNKNOWN, 0x808, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define ioctl_write_memory CTL_CODE(FILE_DEVICE_UNKNOWN, 0x809, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define ioctl_protect_virutal_memory CTL_CODE(FILE_DEVICE_UNKNOWN, 0x810, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define ioctl_allocate_virtual_memory CTL_CODE(FILE_DEVICE_UNKNOWN, 0x811, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define ioctl_get_module_base CTL_CODE(FILE_DEVICE_UNKNOWN, 0x812, METHOD_BUFFERED, FILE_SPECIAL_ACCESS) 
#define ioctl_get_table_module_base CTL_CODE(FILE_DEVICE_UNKNOWN, 0x813, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define ioctl_write_protected_memory CTL_CODE(FILE_DEVICE_UNKNOWN, 0x814, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

typedef struct _k_get_base_module_request {
	UINT64 pid;
	UINT64 handle;
	WCHAR name[260];
} k_get_base_module_request, * pk_get_base_module_request;

typedef struct _k_get_table_module_base_request {
	UINT64 handle;
} k_get_table_module_base_request, * pk_get_table_module_base_request;

typedef struct _k_rw_request {
	UINT64 pid;
	UINT64 src;
	UINT64 dst;
	UINT64 size;
} k_rw_request, * pk_rw_request;

typedef struct _k_alloc_mem_request {
	UINT64 pid, allocation_type, protect;
	UINT64 addr;
	UINT64 size;
} k_alloc_mem_request, * pk_alloc_mem_request;

typedef struct _k_protect_mem_request {
	UINT64 pid, protect;
	UINT64 addr;
	UINT64 size;
} k_protect_mem_request, * pk_protect_mem_request;

typedef struct _k_rw_protected_request {
	UINT64 pid;
	UINT64 src;
	UINT64 dst;
	UINT64 size;
} k_rw_protected_request, * pk_rw_protected_request;

class kernelmode_proc_handler final {
	HANDLE handle;
	uint32_t pid;
public:
	kernelmode_proc_handler();

	~kernelmode_proc_handler();

	bool is_attached();

	bool attach(const char* proc_name);

	UINT64 get_module_base(const std::string &module_name);

	UINT64 get_table_module_base();

	void read_memory(UINT64 src, UINT64 dst, UINT64 size);

	void write_memory(UINT64 dst, UINT64 src, UINT64 size);

	UINT64 virtual_protect(UINT64 address, UINT64 size, UINT64 protect);

	UINT64 virtual_alloc(UINT64 size, UINT64 allocation_type, UINT64 protect, UINT64 address = NULL);

	void write_protected_memory(UINT64 dst, UINT64 src, UINT64 size);
};