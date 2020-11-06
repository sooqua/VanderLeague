#include "HvppIoctl.h"

#include "Debug.h"
#include "../hvpp/src/hvpp/hvpp/lib/ioctl.h"


typedef struct ioctl_write_protected_memory_request_t {
	uint32_t src;
	uint32_t dst;
	uint32_t size;
} ioctl_write_protected_memory_request;

using ioctl_write_protected_memory_t = ioctl_read_write_t<1, sizeof(ioctl_write_protected_memory_request_t)>;

HvppIoctl::HvppIoctl()
{
	DeviceHandle = CreateFile(TEXT("\\\\.\\hvpp"),
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (DeviceHandle == INVALID_HANDLE_VALUE)
	{
		debug::flog("Error while opening 'hvpp' device!\n");
		initialized = false;
		return;
	}

	initialized = true;
}

HvppIoctl::~HvppIoctl()
{
	if (initialized) {
		CloseHandle(DeviceHandle);
	}
}

void HvppIoctl::WriteProtectedMemory(PVOID src, PVOID dst, SIZE_T size)
{
	if (!initialized) {
		return;
	}
	
	ioctl_write_protected_memory_request req = { 0 };
	req.src = (uint32_t)src;
	req.dst = (uint32_t)dst;
	req.size = size;

	DWORD BytesReturned;
	DeviceIoControl(
		DeviceHandle,
		ioctl_write_protected_memory_t::code,
		&req,
		sizeof(req),
		&req,
		sizeof(req),
		&BytesReturned,
		NULL);
}
