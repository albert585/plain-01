#include "idt.h"

#include <stdint.h>
#include "arch/x64/drivers/serial.h"
#include "arch/x64/drivers/framebuffer.h"
struct InterruptDescriptor64 {
    uint16_t offset_1;        // offset bits 0..15
    uint16_t selector;        // a code segment selector in GDT or LDT
    uint8_t  ist;             // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
    uint8_t  type_attributes; // gate type, dpl, and p fields
    uint16_t offset_2;        // offset bits 16..31
    uint32_t offset_3;        // offset bits 32..63
    uint32_t zero;            // reserved
}__attribute__((packed));
struct InterruptDescriptor64 idt[256];
struct idtr_t{
    uint16_t size;
    uint64_t offset;
}__attribute__((packed));
struct idtr_t idtr={sizeof(idt)-1,(uint64_t)idt};

uint64_t ms=0;
uint64_t second=0;

void set_idt_entry(uint8_t vec, void (*handler)(void), uint16_t selector, uint8_t type_attr,uint8_t ist){
    uint64_t addr = (uint64_t)handler;
    idt[vec].offset_1=addr&0xFFFF;
    idt[vec].offset_2=(addr>>16)&0xFFFF;
    idt[vec].offset_3=(addr>>32)&0xFFFFFFFF;
    idt[vec].selector=selector;
    idt[vec].type_attributes=type_attr;
    idt[vec].ist=ist;
}
void load_idt(void)
{
    asm volatile("lidt %0" :: "m"(idtr) : "memory");
}

void interrupt_handler(void){
    ms++;

    if(!(ms%1000)){second++;}
    return;

}
static void print_hex64(uint64_t val,uint8_t * fb,uint32_t x,uint32_t y){
    char buf[18];
    for(int i=15;i>=0;--i){
        uint8_t temp = val& 0xF;
        buf[i]=temp<10 ? '0'+temp:'A'+(temp-10);
        val>>=4;
    }
    buf[16]='\0';
   draw_string(fb,x,y,&buf[0]);
}
void  division_error_handler(void){
    uint8_t *fb=(uint8_t*)framebuffer.response->framebuffers[0]->address;
    draw_string(fb,500,500,"Error: divided by zero");

}
void  double_fault_handler(uint64_t error_code,uint64_t rip){
    uint8_t *fb=(uint8_t*)framebuffer.response->framebuffers[0]->address;

    struct RGBA c={0,0,0};
    for(uint32_t y=0; y<framebuffer.response->framebuffers[0]->height;++y){
        for(uint32_t x=0;x<framebuffer.response->framebuffers[0]->width;++x){
            draw_pixel(fb,x,y,c);}}
            draw_string(fb,500,500,"DOUBLE FAULT");
            draw_string(fb,500,517,"ERROR CODE:");
            print_hex64(error_code,fb,589,517);
            draw_string(fb,500,534,"RIP:0x");
            print_hex64(rip,fb,549,534);
}
void  tss_fault_handler(uint64_t error_code,uint64_t rip){
    uint8_t *fb=(uint8_t*)framebuffer.response->framebuffers[0]->address;

    struct RGBA c={0,0,0};
    for(uint32_t y=0; y<framebuffer.response->framebuffers[0]->height;++y){
        for(uint32_t x=0;x<framebuffer.response->framebuffers[0]->width;++x){
            draw_pixel(fb,x,y,c);}}
            draw_string(fb,500,500,"Invalid TSS");
            draw_string(fb,500,517,"ERROR CODE:");
            print_hex64(error_code,fb,589,517);
            draw_string(fb,500,534,"RIP:0x");
            print_hex64(rip,fb,549,534);
}

void  page_fault_handler(uint64_t error_code,uint64_t rip){
    uint8_t *fb=(uint8_t*)framebuffer.response->framebuffers[0]->address;

    struct RGBA c={0,0,0};
    for(uint32_t y=0; y<framebuffer.response->framebuffers[0]->height;++y){
        for(uint32_t x=0;x<framebuffer.response->framebuffers[0]->width;++x){
            draw_pixel(fb,x,y,c);}}
            draw_string(fb,500,500,"PAGE FAULT");
            draw_string(fb,500,517,"ERROR CODE:");
            print_hex64(error_code,fb,589,517);
            draw_string(fb,500,534,"RIP:0x");
            print_hex64(rip,fb,549,534);
}
static char scancode_to_ascii[128] = {
    0,   0,   '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', 0,   0,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '[', ']', 0,   0,   'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0,   '*',
    0,   ' ', 0,   0,   0,   0,   0,   0,
};

void keyboard_irq_handler(uint64_t scancode) {
    // 忽略释放键（0x80 位）
    if (scancode & 0x80) return;

    // 查表
    char ch = scancode_to_ascii[scancode];
    if (ch) {
        write_serial(ch);  // 输出到串口
        // 如果你有 VGA 输出，也可以 `vga_putchar(ch)`;
    }
}
