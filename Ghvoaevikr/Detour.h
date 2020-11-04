#pragma once
//#include <Windows.h>
//#include <Zydis/Zydis.h>
//#include <Zydis/Decoder.h>

//struct HookContext
//{
//	BYTE original_code[64];
//	SIZE_T dst_ptr;
//	BYTE far_jmp[6];
//};
//
//HookContext* presenthook64;

//const unsigned int DisasmLengthCheck(const BYTE* address, const unsigned int jumplength)
//{
//	ZydisDecoder decoder;
//	ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_COMPAT_32, ZYDIS_ADDRESS_WIDTH_32);
//
//	ZydisDecodedInstruction instruction;
//
//	unsigned int processed = 0;
//	auto EIP = address;
//	while (processed < jumplength)
//	{
//		auto status = ZydisDecoderDecodeBuffer(&decoder, EIP, 0, &instruction);
//		const int len = instruction.length;
//		if (ZYAN_FAILED(status))
//		{
//			++EIP;
//		}
//		else
//		{
//			processed += len;
//			EIP += len;
//		}
//	}
//
//	return processed;
//}
//
//extern void* detourBuffer[3];
//
//const void* DetourFunc(BYTE* const src, const BYTE* dest, const DWORD length);



