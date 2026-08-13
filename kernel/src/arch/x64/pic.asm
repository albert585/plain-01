global init_pic
align   4
init_pic:
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

    mov al,0x11;
    out 0x20,al;
    mov al, 0x20
    out 0x21, al
    mov al, 0x04
    out 0x21, al
    mov al, 0x01
    out 0x21, al

    mov al, 0x20
    out 0x21, al
    mov al, 0x04
    out 0x21, al
    mov al, 0x01
    out 0x21, al
    mov al, 0x11
    out 0xA0, al            ; 写入从 PIC 命令端口

    ; 2. 发送 ICW2: 设置中断向量基址为 0x28 (IRQ8 -> 中断 40)
    mov al, 0x28
    out 0xA1, al            ; 写入从 PIC 数据端口

    ; 3. 发送 ICW3: 告诉从 PIC，自己的编号是 2 (连接在主片的 IRQ2)
    mov al, 0x02
    out 0xA1, al

    ; 4. 发送 ICW4: 设置为 8086/88 模式
    mov al, 0x01
    out 0xA1, al

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
    ret
