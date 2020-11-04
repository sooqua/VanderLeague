#include "Hvpp.h"

#include "Debug.h"
#include "ColdHook.h"
#include <Zydis/Zydis.h>
#include <Zydis/Decoder.h>

#include "hvpp/ia32/asm.h"
#include "hvpp/lib/mp.h"

#define PAGE_SIZE       4096
#define PAGE_ALIGN(Va)  ((PVOID)((ULONG_PTR)(Va) & ~(PAGE_SIZE - 1)))

static void Disassemble(PVOID address) {
	ZydisDecoder decoder;
	ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_COMPAT_32, ZYDIS_ADDRESS_WIDTH_32);

	ZydisFormatter formatter;
	ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);

	ZydisDecodedInstruction instruction;

	unsigned int processed = 0;
	auto EIP = (PBYTE)address;
	while (processed < 6)
	{
		auto status = ZydisDecoderDecodeBuffer(&decoder, EIP, ZYDIS_MAX_INSTRUCTION_LENGTH, &instruction);
		if (ZYAN_FAILED(status))
		{
			++EIP;
		}
		else
		{
			EIP += instruction.length;
			processed += 1;

			char buffer[256];
			ZydisFormatterFormatInstruction(&formatter, &instruction, buffer, sizeof(buffer), (ZyanU64)EIP);
			buffer[255] = '\0';
			debug::flog("%s\n", buffer);
		}
	}
}

void Hvpp::Hook()
{
	debug::flog("\nBefore DetourHook\n");
	Disassemble(OriginalFunction);

	//memset(detourBuffer, 0, sizeof(detourBuffer) / sizeof(void*));
	//DetourFunc((PBYTE)OriginalFunction, (PBYTE)FunctionToHook, length);

	if (!ColdHook_Service::ServiceGlobalInit(nullptr)) {
		debug::flog("Could not initialize ColdHook global service\n");
	}
	auto hookId = ColdHook_Service::InitFunctionHookByAddress(&hookInfo, true, (void*)OriginalFunction, (void*)FunctionToHook, nullptr);
	if (!hookId) {
		debug::flog("Could not initialize function hook by address\n");
	}
	ColdHook_Service::ServiceRegisterHookInformation(hookInfo, hookId, nullptr);

	debug::flog("\nAfter DetourHook\n");
	Disassemble(OriginalFunction);
}

void Hvpp::Hide()
{
	debug::flog("\nBefore Hide\n");
	Disassemble(OriginalFunction);

	struct CONTEXT { void* PageRead; void* PageExecute; } Context{ OriginalFunctionBackupAligned, OriginalFunctionAligned };
	debug::flog("PageRead = %p\n\n", Context.PageRead);
	debug::flog("PageExecute = %p\n\n", Context.PageExecute);
	ForEachLogicalCore([](void* ContextPtr) {
		CONTEXT* Context = (CONTEXT*)ContextPtr;
		ia32_asm_vmx_vmcall(0xc1, (uint32_t)Context->PageRead, (uint32_t)Context->PageExecute);
		}, &Context);

	debug::flog("\nAfter Hide\n");
	Disassemble(OriginalFunction);
}

void Hvpp::Unhide()
{
	debug::flog("\nBefore Unhide\n");
	Disassemble(OriginalFunction);

	ForEachLogicalCore([](void*) { ia32_asm_vmx_vmcall(0xc2, 0, 0); }, nullptr);

	VirtualUnlock(OriginalFunctionAligned, PAGE_SIZE);
	VirtualUnlock(OriginalFunctionBackupAligned, PAGE_SIZE);
	free(OriginalFunctionBackup);

	debug::flog("\nAfter Unhide\n");
	Disassemble(OriginalFunction);
}

void* Hvpp::GetOriginalAddress()
{
	return hookInfo->OriginalF;
}

Hvpp::Hvpp(PVOID src, PVOID dst) {
	OriginalFunction = src;
	FunctionToHook = dst;
	OriginalFunctionAligned = PAGE_ALIGN(OriginalFunction);

	OriginalFunctionBackup = malloc(PAGE_SIZE * 2);
	OriginalFunctionBackupAligned = PAGE_ALIGN((ULONG_PTR)OriginalFunctionBackup + PAGE_SIZE);
	memcpy(OriginalFunctionBackupAligned, OriginalFunctionAligned, PAGE_SIZE);

	debug::flog("OriginalFunction                  : 0x%p\n", OriginalFunction);
	debug::flog("OriginalFunctionAligned           : 0x%p\n", OriginalFunctionAligned);
	debug::flog("OriginalFunctionBackup            : 0x%p\n", OriginalFunctionBackup);
	debug::flog("OriginalFunctionBackupAligned     : 0x%p\n", OriginalFunctionBackupAligned);
	debug::flog("FunctionToHook                    : 0x%p\n", FunctionToHook);
	debug::flog("\n");

	VirtualLock(OriginalFunctionAligned, PAGE_SIZE);
	VirtualLock(OriginalFunctionBackupAligned, PAGE_SIZE);
}