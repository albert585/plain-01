; from OSDev
global reloadSegments
reloadSegments:
   ; Reload CS register:
   PUSH 0x08                 ; 将代码段推送到堆栈，0x08代表是你的代码段
   LEA RAX, [rel .reload_CS] ; 将.reload_CS的地址加载到RAX中
   PUSH RAX                  ; 将此值推入栈
   RETFQ                     ; 根据语法执行far return、RETFQ或LRETQ
.reload_CS:
   ; 重新加载数据段寄存器
   MOV   AX, 0x10 ; 0x10代表是你的数据段
   MOV   DS, AX
   MOV   ES, AX
   MOV   FS, AX
   MOV   GS, AX
   MOV   SS, AX
   RET
