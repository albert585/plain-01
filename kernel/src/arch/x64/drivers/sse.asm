align 4
extern err_not_support
global sse_start
extern hang
sse_start: ;adapt from OSDev.org
    push rax
    push rdx
    push rbx
    push rcx
    mov rax,0x1
    cpuid
    test rdx,1<<25
    jz .noSSE
    mov rax,cr0
    and ax,0xFFFB
    or ax,0x2
    mov cr0,rax
    mov rax,cr4
    or ax,3<<9
    mov cr4,rax
    pop rcx
    pop rbx
    pop rdx
    pop rax
    ret
.noSSE:
    mov rdi,1
    cld
    call err_not_support
    jmp hang


hang:
    hlt
    jmp hang
