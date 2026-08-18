/*
 * Ported from iNagisa/OS_design (https://github.com/iNagisa/OS_design)
 * Copyright (C) iNagisa. Licensed under 0BSD.
 * Adapted for plain-01.
 */

#pragma once

#include <stdint.h>

/* ---- 硬件标志位（x86-64 PTE） ---- */
#define PTE_PRESENT  (1ULL << 0)
#define PTE_WRITABLE (1ULL << 1)
#define PTE_USER     (1ULL << 2)
#define PTE_HUGE     (1ULL << 7)
#define PTE_NX       (1ULL << 63)

/* 项里"物理地址"部分的掩码（bit12..51） */
#define PTE_ADDR_MASK 0x000FFFFFFFFFF000ULL

/* ---- 通用权限标志（给 map_page 的 flags 参数） ---- */
#define PF_READ  (1UL << 0)   /* x86 没有"禁止读"，此位仅作占位，恒可读 */
#define PF_WRITE (1UL << 1)
#define PF_EXEC  (1UL << 2)
#define PF_USER  (1UL << 3)

/*
 * 把通用权限标志翻译成硬件 PTE 标志。
 * 注意 x86-64 默认可执行，"不可执行"才需要置 NX。
 */
uint64_t pt_arch_flags(uint64_t flags);

/*
 * 在 pml4（虚拟地址指针，指向一张 4K 对齐的 PML4）里
 * 建立 vaddr -> paddr 的 4K 映射。缺的中间页表会自动
 * 用 frame_alloc 分配并清零。
 */
void map_page(uint64_t *pml4, uint64_t vaddr, uint64_t paddr, uint64_t flags);

/*
 * 解除 vaddr 的映射。
 * 注意：当前实现只清 PTE，不释放目标物理页（映射 MMIO/共享页时安全）。
 */
void unmap_page(uint64_t *pml4, uint64_t vaddr);

/* 读 CR3 并经 HHDM 换算，返回当前 PML4 的虚拟地址 */
uint64_t *get_current_pml4(void);

/* 读取 CR2（#PF 出错地址），给 page fault handler 用 */
uint64_t get_cr2(void);
