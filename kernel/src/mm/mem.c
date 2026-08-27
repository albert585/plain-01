#include "mm/mem.h"

#include <limine.h>
#include <stdint.h>

/* Limine 内存地图请求：用来在可用 RAM 上取物理页，供手动建页表用 */
__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = LIMINE_MEMMAP_REQUEST;

static uint64_t hhdm_base = 0;      /* Limine HHDM offset */
static uint64_t next_free_phys = 0; /* 下一个可分配的物理页（bump） */

uint64_t get_cr3(void) {
    uint64_t cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

uint64_t get_hhdm_base(void) {
    return hhdm_base;
}

/* 分配一个已清零的 4K 物理页（bump 分配），用于垫页表。
 * 返回物理地址；访问它时要用 HHDM 虚拟地址（phys + get_hhdm_base()）。 */
uint64_t pmm_alloc_zero(void) {
    uint64_t phys = next_free_phys;
    next_free_phys += 0x1000;

    uint64_t *v = (uint64_t *)(phys + hhdm_base);
    for (int i = 0; i < 512; i++) {
        v[i] = 0;
    }
    return phys;
}

void mm_init(uint64_t hhdm_offset) {
    hhdm_base = hhdm_offset;

    /* 从第一段 >= 4MiB 的 usable RAM 且地址不低于 8MiB 处起分配，
     * 8MiB 起远离 Limine 内核镜像与低端保留区。 */
    uint64_t start = 0x800000; /* 兜底：response 为 NULL 时用固定 8MiB */

    if (memmap_request.response) {
        for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
            struct limine_memmap_entry *e =
                memmap_request.response->entries[i];
            if (e->type != LIMINE_MEMMAP_USABLE || e->length < 0x400000) {
                continue;
            }
            uint64_t b = (e->base > 0x800000) ? e->base : 0x800000;
            if (b + 0x1000 <= e->base + e->length) {
                start = b;
                break;
            }
        }
    }
    next_free_phys = start;
}
