#include "idt.h"
#include <stdint.h>
#include "arch/x64/drivers/serial.h"
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
void set_idt_entry(uint8_t vec, void (*handler)(void), uint16_t selector, uint8_t type_attr){
    uint64_t addr = (uint64_t)handler;
    idt[vec].offset_1=addr&0xFFFF;
    idt[vec].offset_2=(addr>>16)&0xFFFF;
    idt[vec].offset_3=(addr>>32)&0xFFFFFFFF;
    idt[vec].selector=selector;
    idt[vec].type_attributes=type_attr;
    idt[vec].ist=0;
}
void load_idt(void)
{
    asm volatile("lidt %0" :: "m"(idtr) : "memory");
}
void interrupt_handler(void){
    serial_printk("INT!");
}
