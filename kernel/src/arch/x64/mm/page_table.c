/*
 * Ported from iNagisa/OS_design (https://github.com/iNagisa/OS_design)
 * Copyright (C) iNagisa. Licensed under 0BSD.
 * Adapted for plain-01.
 */

#include <arch/x64/mm/page_table.h>
#include <mm/hhdm.h>
#include <mm/frame.h>
#include <lib/memory.h>

uint64_t pt_arch_flags(uint64_t flags)
{
    uint64_t pte = PTE_PRESENT;

    if (flags & PF_WRITE)
        pte |= PTE_WRITABLE;

    if (flags & PF_USER)
        pte |= PTE_USER;

    if (!(flags & PF_EXEC))
        pte |= PTE_NX;

    return pte;
}

static uint64_t *walk_or_alloc(uint64_t *table, uint64_t index, uint64_t arch_flags)
{
    uint64_t entry = table[index];

    if (!(entry & PTE_PRESENT))
    {
        uint64_t new_phys = frame_alloc(1);
        if (new_phys == 0)
            return 0; /* 物理内存耗尽 */

        uint64_t *new_table = (uint64_t *)phys_to_virt(new_phys);
        kmemset(new_table, 0, PAGE_SIZE_4K);

        /* 中间层：P | W，USER 位从 flags 继承 */
        table[index] = new_phys | PTE_PRESENT | PTE_WRITABLE | (arch_flags & PTE_USER);
        return new_table;
    }

    return (uint64_t *)phys_to_virt(entry & PTE_ADDR_MASK);
}

void map_page(uint64_t *pml4, uint64_t vaddr, uint64_t paddr, uint64_t flags)
{
    uint64_t indices[4] = {
        (vaddr >> 39) & 0x1FF, /* PML4 */
        (vaddr >> 30) & 0x1FF, /* PDPT */
        (vaddr >> 21) & 0x1FF, /* PD   */
        (vaddr >> 12) & 0x1FF, /* PT   */
    };

    uint64_t arch = pt_arch_flags(flags);
    uint64_t *table = pml4;

    for (int level = 0; level < 3; ++level)
    {
        table = walk_or_alloc(table, indices[level], arch);
        if (!table)
            return; /* 分配失败，映射未建立 */
    }

    table[indices[3]] = (paddr & PTE_ADDR_MASK) | arch;

    /* 新映射立即生效 */
    asm volatile("invlpg (%0)" ::"r"(vaddr) : "memory");
}

void unmap_page(uint64_t *pml4, uint64_t vaddr)
{
    uint64_t *table = pml4;
    uint64_t index;

    for (int level = 0; level < 3; ++level)
    {
        index = (vaddr >> (39 - 9 * level)) & 0x1FF;
        uint64_t entry = table[index];
        if (!(entry & PTE_PRESENT))
            return; /* 页表不存在，无需 unmap */

        table = (uint64_t *)phys_to_virt(entry & PTE_ADDR_MASK);
    }

    index = (vaddr >> 12) & 0x1FF;
    table[index] = 0; /* 只清映射，不 free 目标物理页 */

    asm volatile("invlpg (%0)" ::"r"(vaddr) : "memory");
}

uint64_t *get_current_pml4(void)
{
    uint64_t cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    return (uint64_t *)phys_to_virt(cr3);
}

uint64_t get_cr2(void)
{
    uint64_t cr2;
    asm volatile("mov %%cr2, %0" : "=r"(cr2));
    return cr2;
}
