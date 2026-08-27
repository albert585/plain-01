global irq1_handler
extern keyboard_irq_handler
irq1_handler:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    cli
    ; 读取键盘扫描码
    in al, 0x60
    movzx rdi,al
    ; 处理扫描码（调用 C 函数）
    call keyboard_irq_handler

    ; 发送 EOI 到主 PIC
    mov al, 0x20
    out 0x20, al
    sti
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    iretq
