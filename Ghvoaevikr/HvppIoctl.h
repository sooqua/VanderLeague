#pragma once

#include <Windows.h>

class HvppIoctl {
	HANDLE DeviceHandle;
	bool initialized;
public:
	HvppIoctl();
	~HvppIoctl();
	void WriteProtectedMemory(PVOID src, PVOID dst, SIZE_T size);
};