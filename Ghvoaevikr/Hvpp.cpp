#include "Hvpp.h"

#include "Debug.h"
#include "ColdHook.h"
#include <Zydis/Zydis.h>
#include <Zydis/Decoder.h>

#include "hvpp/ia32/asm.h"
#include "hvpp/lib/mp.h"

#define PAGE_SIZE       4096
#define PAGE_ALIGN(Va)  ((PVOID)((ULONG_PTR)(Va) & ~(PAGE_SIZE - 1)))

void Hvpp::Disassemble() {
	ZydisDecoder decoder;
	ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_COMPAT_32, ZYDIS_ADDRESS_WIDTH_32);

	ZydisFormatter formatter;
	ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);

	ZydisDecodedInstruction instruction;

	unsigned int processed = 0;
	auto EIP = (PBYTE)OriginalFunction;
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
	auto hookId = ColdHook_Service::InitFunctionHookByAddress(&hookInfo, true, (void*)OriginalFunction, (void*)FunctionToHook, nullptr);
	if (!hookId) {
		debug::flog("Could not initialize function hook by address\n");
	}
	ColdHook_Service::ServiceRegisterHookInformation(hookInfo, hookId, nullptr);
}

void Hvpp::Hide()
{
	struct CONTEXT { void* PageRead; void* PageExecute; } Context{ OriginalFunctionBackupAligned, OriginalFunctionAligned };
	ForEachLogicalCore([](void* ContextPtr) {
		CONTEXT* Context = (CONTEXT*)ContextPtr;
		ia32_asm_vmx_vmcall(0xc1, (uint32_t)Context->PageRead, (uint32_t)Context->PageExecute);
		}, &Context);
}

void Hvpp::GlobalUnhide()
{
	ForEachLogicalCore([](void*) { ia32_asm_vmx_vmcall(0xc2, 0, 0); }, nullptr);
}

void* Hvpp::GetOriginalAddress()
{
	return hookInfo->OriginalF;
}

void Hvpp::Init(PVOID src, PVOID dst) {
	OriginalFunction = src;
	FunctionToHook = dst;
	OriginalFunctionAligned = PAGE_ALIGN(OriginalFunction);

	OriginalFunctionBackup = malloc(PAGE_SIZE * 2);
	OriginalFunctionBackupAligned = PAGE_ALIGN((ULONG_PTR)OriginalFunctionBackup + PAGE_SIZE);
	memcpy(OriginalFunctionBackupAligned, OriginalFunctionAligned, PAGE_SIZE);

	VirtualLock(OriginalFunctionAligned, PAGE_SIZE);
	VirtualLock(OriginalFunctionBackupAligned, PAGE_SIZE);

	initialized = true;
}

Hvpp::Hvpp() {
	OriginalFunction = nullptr;
	OriginalFunctionAligned = nullptr;
	OriginalFunctionBackup = nullptr;
	OriginalFunctionBackupAligned = nullptr;
	FunctionToHook = nullptr;
	hookInfo = nullptr;

	initialized = false;
}

Hvpp::~Hvpp() {
	if (initialized) {
		VirtualUnlock(OriginalFunctionAligned, PAGE_SIZE);
		VirtualUnlock(OriginalFunctionBackupAligned, PAGE_SIZE);
		free(OriginalFunctionBackup);
	}
}
