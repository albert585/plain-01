

#ifndef PLAIN_MM_PAGING_H
#define PLAIN_MM_PAGING_H

#include <stddef.h>
#include <stdint.h>

#include "mm/mem.h" /* P_PRESENT, P_WRITE, P_USER, MMIO_VBASE,
                        pmm_alloc_zero, get_hhdm_base, get_cr3 */


/* 一个规范的 4 级虚拟地址：按索引 PML4 / PDPT / PD / PT / 页内偏移的
 * 5 个字段切开，直接给 union 赋值即可读各段。 */
union LinearAddress4Level {
  uint64_t value; /* 整条 64 位虚拟地址 */

  struct {
    uint64_t offset : 12; /* [11:0]  页内偏移（4K 页 = 12 位） */
    uint64_t page : 9;    /* [20:12] PT 表索引，512 个表项 */
    uint64_t dir : 9;     /* [29:21] PD 表索引 */
    uint64_t pdp : 9;     /* [38:30] PDPT 表索引 */
    uint64_t pml4 : 9;    /* [47:39] PML4 表索引 */
    uint64_t : 16;        /* [63:48] 高位，需符号扩展为规范地址 */
  } __attribute__((packed)) parts;
};

/* 单个 64 位页表项：低位是标志位，高位存下一级表的物理页框号。 */
union PageMapEntry {
  uint64_t data; /* 整项的原始值 */

  struct {
    uint64_t present : 1;       /* P    1=有效 */
    uint64_t writable : 1;      /* R/W  1=可写 */
    uint64_t user : 1;          /* U/S  1=用户态可访问 */
    uint64_t write_through : 1; /* PWT  写穿缓存 */
    uint64_t cache_disable : 1; /* PCD  禁用该页缓存 */
    uint64_t accessed : 1;      /* A    CPU 置位：已被访问 */
    uint64_t dirty : 1;         /* D    CPU 置位：已被写过 */
    uint64_t huge_page : 1;     /* PS   1=大页（2M/1G，本内核未用） */
    uint64_t global : 1;        /* G    全局页，不随 CR3 切换失效 */
    uint64_t : 3;               /*      [11:9] 保留/软件可用 */

    uint64_t addr : 40; /* [51:12] 下一级表或数据页的物理页框号 */
    uint64_t : 12;      /* [63:52] 保留，NX 位另见 mem.h 的 P_NX */
  } __attribute__((packed)) bits;
};

/* 取第 level 段的索引。
 * level 0=offset, 1=PT, 2=PD, 3=PDPT, 4=PML4 */
static inline int addr_part(const union LinearAddress4Level a, int level) {
  switch (level) {
    case 0: return (int)a.parts.offset;
    case 1: return (int)a.parts.page;
    case 2: return (int)a.parts.dir;
    case 3: return (int)a.parts.pdp;
    case 4: return (int)a.parts.pml4;
    default: return 0;
  }
}


static inline void addr_set_part(union LinearAddress4Level *a, int level, int v) {
  switch (level) {
    case 0: a->parts.offset = (uint64_t)v; break;
    case 1: a->parts.page = (uint64_t)v; break;
    case 2: a->parts.dir = (uint64_t)v; break;
    case 3: a->parts.pdp = (uint64_t)v; break;
    case 4: a->parts.pml4 = (uint64_t)v; break;
  }
}

/*
 * 注意：这里存的是【物理地址】——Limine 下虚拟地址 != 物理地址，
 * HHDM 偏移只在解引用页表时加，写入页表项时绝不加。 */
static inline uint64_t entry_phys(const union PageMapEntry e) {
  return e.bits.addr << 12; /* 页框号左移 12 位还原成物理地址 */
}

static inline void entry_set_phys(union PageMapEntry *e, uint64_t phys) {
  e->bits.addr = phys >> 12; /* 物理地址右移 12 位，只留页框号 */
}

/* 把从 paddr_first 起的 pages 个连续 4K 物理页，映射到从 vaddr 起的
 * pages 个连续 4K 虚拟页上；中间级页表缺失时用 pmm_alloc_zero() 现分配，
 * 并经 HHDM 访问。flags 用 mem.h 里的 P_* 组合。 */
void map_pages(uint64_t vaddr, uint64_t paddr_first, size_t pages,
               uint64_t flags);

/* 把一个设备 BAR（占 pages 个 4K 页）映射到一块新的高半区地址，
 * 返回该虚拟基址。pages 要覆盖整个 BAR（如 xHCI 的 16K => 4）。 */
uint8_t *mmio_map(uint64_t phys_page, size_t pages);

#endif /* PLAIN_MM_PAGING_H */
