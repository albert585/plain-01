#pragma once
#include <stdint.h>
extern void isr_wrapper(void);
extern void double_fault_wrapper(void);
void set_idt_entry(uint8_t vec, void (*handler)(void), uint16_t selector, uint8_t type_attr,uint8_t isr);
void load_idt(void);
void interrupt_handler(void);
void double_fault_handler(uint64_t error_code,uint64_t rip);
