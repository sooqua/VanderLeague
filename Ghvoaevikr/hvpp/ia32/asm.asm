;++
;
; Copyright (c) Petr Benes. All rights reserved.
;
; Module:
;
;   asm.asm
;
; Abstract:
;
;   Contains functions to access Intel CPU instructions which are not exported
;   by intrin.h.
;
; Author:
;
;    Petr Benes (@PetrBenes) 26-Jul-2018 - Initial version
;
; Environment:
;
;    Kernel mode only.
;
;--

.686p
.XMM
.model flat, stdcall

.code
    ia32_asm_vmx_vmcall PROC hypercall_number, page_read, page_execute
        mov ecx, hypercall_number
        mov edx, page_read
        mov ebx, page_execute
        vmcall
        ret
    ia32_asm_vmx_vmcall ENDP
end
