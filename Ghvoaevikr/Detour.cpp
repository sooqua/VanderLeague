//#include "Detour.h"
//
//const void* DetourFunc(BYTE* const src, const BYTE* dest, const DWORD length)
//{
//	BYTE* jump = new BYTE[length + 5];
//	for (int i = 0; i < sizeof(detourBuffer) / sizeof(void*); ++i)
//	{
//		if (!detourBuffer[i])
//		{
//			detourBuffer[i] = jump;
//			break;
//		}
//	}
//
//	DWORD dwVirtualProtectBackup;
//	VirtualProtect(src, length, PAGE_READWRITE, &dwVirtualProtectBackup);
//
//	memcpy(jump, src, length);
//	jump += length;
//
//	jump[0] = 0xE9;
//	*(DWORD*)(jump + 1) = (DWORD)(src + length - jump) - 5;
//
//	src[0] = 0xE9;
//	*(DWORD*)(src + 1) = (DWORD)(dest - src) - 5;
//
//	VirtualProtect(src, length, dwVirtualProtectBackup, &dwVirtualProtectBackup);
//
//	return jump - length;
//}
