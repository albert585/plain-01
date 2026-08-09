#include <arch/x64/x64.h>
#include <lib/strings.h>
void serial_init() {
    outb(0x3F8 + 1, 0x00);   // 关中断
    outb(0x3F8 + 3, 0x80);   // DLAB=1, 准备配波特率
    outb(0x3F8 + 0, 0x03);   // 115200 / 3 = 38400 低位
    outb(0x3F8 + 1, 0x00);   // 高位 = 0
    outb(0x3F8 + 3, 0x03);   // 8N1, DLAB=0
    outb(0x3F8 + 2, 0xC7);   // 开 FIFO, 清发送/接收
}

char read_serial()
{
    while ((inb(SERIAL_PORT + 5) & 1) == 0);
    return inb(SERIAL_PORT);
}

void write_serial(char a)
{
    while ((inb(SERIAL_PORT + 5) & 0x20) == 0)
        ;
    outb(SERIAL_PORT, a);
}

void serial_printk(char *buf)
{
    for (int i = 0; i < kstrlen(buf); i++)
    {
        if (buf[i] == '\n')
            write_serial('\r');
        write_serial(buf[i]);
    }
}
