#include "kernelmode_proc_handler.hpp"

#include <thread>

kernelmode_proc_handler::kernelmode_proc_handler() 
	:handle{ INVALID_HANDLE_VALUE }, pid{ 0 } {}

kernelmode_proc_handler::~kernelmode_proc_handler() { if (is_attached()) CloseHandle(handle); }

bool kernelmode_proc_handler::is_attached() { return handle != INVALID_HANDLE_VALUE; }

bool kernelmode_proc_handler::attach(const char* proc_name) {
	bool is_admin{ false };
	HANDLE token_handle{ NULL };
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token_handle)) {
		TOKEN_ELEVATION token;
		DWORD size = sizeof(TOKEN_ELEVATION);
		if (GetTokenInformation(token_handle, TokenElevation, &token, sizeof(TOKEN_ELEVATION), &size)) {
			is_admin = true;
		}
		CloseHandle(token_handle);
	}

	if (!is_admin) {
		LOG_ERROR("Launch as admin");
		return false;
	}

	while (!is_process_running(proc_name, pid))
		std::this_thread::sleep_for(std::chrono::seconds(1));

	handle = CreateFileA("\\\\.\\injdrv", GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);

	if (handle == INVALID_HANDLE_VALUE) {
		LOG_ERROR("Load the driver first");
		return false;
	}

	return true;
};

UINT64 kernelmode_proc_handler::get_module_base(const std::string& module_name) {
	if (handle == INVALID_HANDLE_VALUE)
		return 0;
	k_get_base_module_request req;
	req.pid = pid;
	req.handle = 0;
	std::wstring wstr{ std::wstring(module_name.begin(), module_name.end()) };
	memset(req.name, 0, sizeof(WCHAR) * 260);
	wcscpy_s(req.name, wstr.c_str());
	DWORD bytes_read;
	if (DeviceIoControl(handle, ioctl_get_module_base, &req,
		sizeof(k_get_base_module_request), &req, sizeof(k_get_base_module_request), &bytes_read, 0)) {
		return req.handle;
	}
	return req.handle;
}

UINT64 kernelmode_proc_handler::get_table_module_base() {
	if (handle == INVALID_HANDLE_VALUE)
		return 0;
	k_get_table_module_base_request req = { 0 };
	DWORD bytes_read;
	if (DeviceIoControl(handle, ioctl_get_table_module_base, &req,
		sizeof(k_get_table_module_base_request), &req, sizeof(k_get_table_module_base_request), &bytes_read, 0)) {
		LOG("Driver returned table module handle: %llu", req.handle);
		return req.handle;
	}
	return req.handle;
}

void kernelmode_proc_handler::read_memory(UINT64 src, UINT64 dst, UINT64 size) {
	if (handle == INVALID_HANDLE_VALUE)
		return;
	k_rw_request request{ pid, src, dst, size };
	DWORD bytes_read;
	DeviceIoControl(handle, ioctl_read_memory, &request, sizeof(k_rw_request), 0, 0, &bytes_read, 0);
}

void kernelmode_proc_handler::write_memory(UINT64 dst, UINT64 src, UINT64 size) {
	if (handle == INVALID_HANDLE_VALUE)
		return;
	DWORD bytes_read;
	k_rw_request request{ pid, src, dst, size };
	auto success = DeviceIoControl(handle, ioctl_write_memory, &request, sizeof(k_rw_request), 0, 0, &bytes_read, 0);
	if (!success)
		LOG("write_memory FAIL");
}

UINT64 kernelmode_proc_handler::virtual_protect(UINT64 address, UINT64 size, UINT64 protect) {
	if (handle == INVALID_HANDLE_VALUE)
		return 0;
	DWORD bytes_read;
	k_protect_mem_request request{ pid, protect, address, size };
	if (DeviceIoControl(handle, ioctl_protect_virutal_memory, &request, sizeof(k_protect_mem_request), &request, sizeof(k_protect_mem_request), &bytes_read, 0))
		return protect;
	LOG("virtual_protect FAIL");
	return 0;
}

UINT64 kernelmode_proc_handler::virtual_alloc(UINT64 size, UINT64 allocation_type, UINT64 protect, UINT64 address) {
	if (handle == INVALID_HANDLE_VALUE)
		return 0;
	DWORD bytes_read;
	k_alloc_mem_request request{ static_cast<UINT64>(pid), (UINT64)(MEM_COMMIT | MEM_RESERVE), protect, address, size };
	if (DeviceIoControl(handle, ioctl_allocate_virtual_memory, &request, sizeof(k_alloc_mem_request), &request, sizeof(k_alloc_mem_request), &bytes_read, 0))
		return request.addr;
	return 0;
}

void kernelmode_proc_handler::write_protected_memory(UINT64 dst, UINT64 src, UINT64 size)
{
	if (handle == INVALID_HANDLE_VALUE)
		return;
	DWORD bytes_read;
	k_rw_protected_request request{ pid, src, dst, size };
	LOG("write_protected_memory dst: %llu, src: %llu, size: %llu", dst, src, size);
	auto success = DeviceIoControl(handle, ioctl_write_protected_memory, &request, sizeof(k_rw_protected_request), 0, 0, &bytes_read, 0);
	if (!success)
		LOG("write_protected_memory FAIL");
}
 
