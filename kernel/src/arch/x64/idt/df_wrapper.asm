; #DF (Double Fault, vector 8) entry point.
;
; A real #DF pushes an error code (always 0), so on entry the stack is:
;   [rsp + 0]  = error code
;   [rsp + 8]  = RIP
;   [rsp + 16] = CS
;   [rsp + 24] = RFLAGS
;   [rsp + 32] = RSP
;   [rsp + 40] = SS
;
; A double fault means the CPU state is no longer trustworthy, so this
; entry point never returns: it reports the error and halts.

global double_fault_wrapper
global division_error_wrapper
global tss_fault_wrapper
extern division_error_handler
extern double_fault_handler
extern tss_fault_handler

align 4


hang:
    hlt
    jmp hang

division_error_wrapper:
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    cld
    call division_error_handler
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax
    add  [rsp],2
    iretq



double_fault_wrapper:
    cld                         ; SysV ABI requires DF = 0 on function entry
    mov rdi, [rsp]
    mov rsi, [rsp+8]; pass error code as 1st argument (rdi)
    call double_fault_handler
    jmp hang

tss_fault_wrapper:
    cld                         ; SysV ABI requires DF = 0 on function entry
    mov rdi, [rsp]
    mov rsi, [rsp+8] ; pass error code as 1st argument (rdi)
    call tss_fault_handler
    jmp hang
