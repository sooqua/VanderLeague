#pragma once
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

#pragma optimize("", off)
	uint32_t __stdcall ia32_asm_vmx_vmcall(uint32_t hypercall_number, uint32_t page_read, uint32_t page_execute);
#pragma optimize("", on)

#ifdef __cplusplus
}
#endif
