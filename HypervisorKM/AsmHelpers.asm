.code _text

extern CLaunchOneCpu: proc
extern MainVmexitHandler: proc
extern VmResumeFailed: proc

STATUS_SUCCESS equ 0
SHADOW_SPACE equ 20h

EXIT_ACTION_RESUME equ 0
EXIT_ACTION_TERMINATE equ 1
EXIT_ACTION_HYPERCALL equ 2

GeneralRegs struct
    rax_ dq ?
    rcx_ dq ?
    rdx_ dq ?
    rbx_ dq ?
    rsp_ dq ? ; placeholder
    rbp_ dq ?
    rsi_ dq ?
    rdi_ dq ?
    r8_ dq ?
    r9_ dq ?
    r10_ dq ?
    r11_ dq ?
    r12_ dq ?
    r13_ dq ?
    r14_ dq ?
    r15_ dq ?
GeneralRegs ends

.errnz (sizeof GeneralRegs) - 80h, <Invalid GuestRegs>
.errnz GeneralRegs.rsp_ - 20h, <RSP offset mismatch>

SAVE_GENERAL_REGISTERS macro
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rdi
    push rsi
    push rbp
    push rbp
    push rbx
    push rdx
    push rcx
    push rax
endm

RESTORE_GENERAL_REGISTERS macro
    pop rax
    pop rcx
    pop rdx
    pop rbx
    lea rsp, [rsp + 8]
    pop rbp
    pop rsi
    pop rdi
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
endm

XmmRegs struct
    xmm0_ oword ?
    xmm1_ oword ?
    xmm2_ oword ?
    xmm3_ oword ?
    xmm4_ oword ?
    xmm5_ oword ?
    xmm6_ oword ?
    xmm7_ oword ?
    xmm8_ oword ?
    xmm9_ oword ?
    xmm10_ oword ?
    xmm11_ oword ?
    xmm12_ oword ?
    xmm13_ oword ?
    xmm14_ oword ?
    xmm15_ oword ?
XmmRegs ends

SAVE_XMM macro
    movaps xmmword ptr [rsp + XmmRegs.xmm0_], xmm0
    movaps xmmword ptr [rsp + XmmRegs.xmm1_], xmm1
    movaps xmmword ptr [rsp + XmmRegs.xmm2_], xmm2
    movaps xmmword ptr [rsp + XmmRegs.xmm3_], xmm3
    movaps xmmword ptr [rsp + XmmRegs.xmm4_], xmm4
    movaps xmmword ptr [rsp + XmmRegs.xmm5_], xmm5
    movaps xmmword ptr [rsp + XmmRegs.xmm6_], xmm6
    movaps xmmword ptr [rsp + XmmRegs.xmm7_], xmm7
    movaps xmmword ptr [rsp + XmmRegs.xmm8_], xmm8
    movaps xmmword ptr [rsp + XmmRegs.xmm9_], xmm9
    movaps xmmword ptr [rsp + XmmRegs.xmm10_], xmm10
    movaps xmmword ptr [rsp + XmmRegs.xmm11_], xmm11
    movaps xmmword ptr [rsp + XmmRegs.xmm12_], xmm12
    movaps xmmword ptr [rsp + XmmRegs.xmm13_], xmm13
    movaps xmmword ptr [rsp + XmmRegs.xmm14_], xmm14
    movaps xmmword ptr [rsp + XmmRegs.xmm15_], xmm15
endm

RESTORE_XMM macro
    movaps xmm0, xmmword ptr [rsp + XmmRegs.xmm0_]
    movaps xmm1, xmmword ptr [rsp + XmmRegs.xmm1_]
    movaps xmm2, xmmword ptr [rsp + XmmRegs.xmm2_]
    movaps xmm3, xmmword ptr [rsp + XmmRegs.xmm3_]
    movaps xmm4, xmmword ptr [rsp + XmmRegs.xmm4_]
    movaps xmm5, xmmword ptr [rsp + XmmRegs.xmm5_]
    movaps xmm6, xmmword ptr [rsp + XmmRegs.xmm6_]
    movaps xmm7, xmmword ptr [rsp + XmmRegs.xmm7_]
    movaps xmm8, xmmword ptr [rsp + XmmRegs.xmm8_]
    movaps xmm9, xmmword ptr [rsp + XmmRegs.xmm9_]
    movaps xmm10, xmmword ptr [rsp + XmmRegs.xmm10_]
    movaps xmm11, xmmword ptr [rsp + XmmRegs.xmm11_]
    movaps xmm12, xmmword ptr [rsp + XmmRegs.xmm12_]
    movaps xmm13, xmmword ptr [rsp + XmmRegs.xmm13_]
    movaps xmm14, xmmword ptr [rsp + XmmRegs.xmm14_]
    movaps xmm15, xmmword ptr [rsp + XmmRegs.xmm15_]
endm

CALL_WITH_GUEST_FRAME macro Handler
    SAVE_GENERAL_REGISTERS
    mov rcx, rsp
    sub rsp, sizeof XmmRegs
    SAVE_XMM
    sub rsp, SHADOW_SPACE
    call Handler
    add rsp, SHADOW_SPACE
    RESTORE_XMM
    add rsp, sizeof XmmRegs
endm

AsmLaunchOneCpu proc public
    pushfq
    sub rsp, sizeof XmmRegs
    SAVE_XMM
    SAVE_GENERAL_REGISTERS

    sub rsp, SHADOW_SPACE
    mov rdx, rcx
    mov rcx, rsp
    call CLaunchOneCpu  

    add rsp, SHADOW_SPACE
    mov qword ptr [rsp + GeneralRegs.rax_], rax
    jmp _RestoreAndReturn
AsmLaunchOneCpu endp

AsmPopState proc public
    add rsp, SHADOW_SPACE    
    mov qword ptr [rsp + GeneralRegs.rax_], STATUS_SUCCESS

_RestoreAndReturn::
    RESTORE_GENERAL_REGISTERS
    RESTORE_XMM
    add rsp, sizeof XmmRegs
    popfq
    ret
AsmPopState endp

make_get_reg macro Reg
	Get&reg&Value proc public
		mov rax, Reg
		ret
	Get&Reg&Value endp
endm

for reg, <RAX, RBX, RCX, RDX, RSI, RDI, RBP, RSP, R8, R9, R10, R11, R12, R13, R14, R15, CS, DS, ES, SS, FS, GS>
	make_get_reg reg
endm

GetGdtBase proc public
	local gdtr[10]: byte
	sgdt gdtr
	mov rax, qword ptr gdtr[2]
	ret
GetGdtBase endp

GetGdtLimit proc public
	local gdtr[10]: byte
	sgdt gdtr
	mov ax, word ptr gdtr[0]
	ret
GetGdtLimit endp

GetLdtr proc public
	sldt rax
	ret
GetLdtr endp

GetTr proc public
	str rax
	ret
GetTr endp

GetIdtBase proc public
	local idtr[10]: byte
	sidt idtr
	mov rax, qword ptr idtr[2]
	ret
GetIdtBase endp

GetIdtLimit proc public
	local idtr[10]: byte
	sidt idtr
	mov ax, word ptr idtr[0]
	ret
GetIdtLimit endp

AsmVmExitHandler proc public
    CALL_WITH_GUEST_FRAME MainVmexitHandler

    cmp al, EXIT_ACTION_TERMINATE
    je AsmVmxoffHandler

    cmp al, EXIT_ACTION_HYPERCALL
    je _ForwardHypercall

    RESTORE_GENERAL_REGISTERS
    jmp _VmResume

_ForwardHypercall:
    RESTORE_GENERAL_REGISTERS
    vmcall

_VmResume:
    vmresume

    CALL_WITH_GUEST_FRAME VmResumeFailed
    jmp AsmVmxoffHandler
AsmVmExitHandler endp

AsmVmxoffHandler proc public
    vmxoff
    
    RESTORE_GENERAL_REGISTERS

    mov rsp, qword ptr [rsp - (sizeof GeneralRegs) + GeneralRegs.rsp_]
    popfq

    ret
AsmVmxoffHandler endp
end