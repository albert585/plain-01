#pragma once
#include <stdint.h>
extern void isr_wrapper(void);
void set_idt_entry(uint8_t vec, void (*handler)(void), uint16_t selector, uint8_t type_attr);
void load_idt(void);
void interrupt_handler(void);
