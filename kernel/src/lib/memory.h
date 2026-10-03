#pragma once
#include <stddef.h>
#include <stdint.h>
#include "mm/paging.h"
#define HEAP_VBASE 0xFFFFD00000000000ULL   /* 空着的 PML4 槽(384 是 MMIO, 256 是 HHDM, 511 是内核) */
static uint64_t heap_cur = HEAP_VBASE, heap_end = HEAP_VBASE + 16 * 0x1000;

void kmemcpy(void *dst,void const * src,size_t n);
void kmemset(void *dst,const char c,size_t n);
void *kmalloc(size_t n);
void kfree(void);
