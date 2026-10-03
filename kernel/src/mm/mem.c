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
    /* CR3 保存当前 PML4 页表的物理地址（低 12 位是标志位，取用时需屏蔽） */
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

uint64_t get_hhdm_base(void) {
    return hhdm_base;
}

/* 分配一个已清零的 4K 物理页（bump 分配），用于垫页表。
 * 返回物理地址；访问它时要用 HHDM 虚拟地址（phys + get_hhdm_base()）。 */
uint64_t pmm_alloc_zero(void) {
    uint64_t phys = next_free_phys; /* 本次拿到的页 */
    next_free_phys += 0x1000;       /* 指针推进一页；不做回收 */

    /* 页表必须全 0 才是"空表"，所以新页一律清零。
     * 512 个 uint64_t = 4096 字节 = 一页（每张页表正好 512 个表项） */
    uint64_t *v = (uint64_t *)(phys + hhdm_base);
    for (int i = 0; i < 512; i++) {
        v[i] = 0;
    }
    return phys;
}

void mm_init(uint64_t hhdm_offset) {
    hhdm_base = hhdm_offset; /* 先把物理↔虚拟的换算关系存下来 */

    /* 从第一段 >= 4MiB 的 usable RAM 且地址不低于 8MiB 处起分配，
     * 8MiB 起远离 Limine 内核镜像与低端保留区。 */
    uint64_t start = 0x800000; /* 兜底：response 为 NULL 时用固定 8MiB */

    if (memmap_request.response) {
        /* 遍历内存地图，找第一段够用（>=4MiB）的可用 RAM */
        for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
            struct limine_memmap_entry *e =
                memmap_request.response->entries[i];
            if (e->type != LIMINE_MEMMAP_USABLE || e->length < 0x400000) {
                continue; /* 非可用内存 / 段太小，跳过 */
            }
            /* 段起点至少抬到 8MiB，避免踩到低端的 Limine 结构 */
            uint64_t b = (e->base > 0x800000) ? e->base : 0x800000;
            /* 抬高后仍要能放下一整页，否则这段不能用 */
            if (b + 0x1000 <= e->base + e->length) {
                start = b;
                break;
            }
        }
    }
    next_free_phys = start; /* bump 指针就位，之后 pmm_alloc_zero 从这里往上发 */
}
