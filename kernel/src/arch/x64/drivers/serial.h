#pragma once

#include <arch/x64/io.h>

#define SERIAL_PORT 0x3f8

void serial_init();

char read_serial();

void write_serial(char ch);

void serial_printk(char *buf, int len);
