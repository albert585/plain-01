

#include "mm/paging.h"

#include <stddef.h>
#include <stdint.h>

#include "mm/mem.h" /* pmm_alloc_zero, get_hhdm_base, get_cr3, P_* */

/* 把【物理】页表地址换算成 Limine 映射到的高半区虚拟地址
 * （HHDM：virt = phys + offset）。页表项里存的是物理地址，解引用前必须过这一步。 */
static union PageMapEntry *table_at(uint64_t phys) {
  return (union PageMapEntry *)(phys + get_hhdm_base());
}

/* 保证 table[idx] 指向一张已存在的下一级页表：若该项 present=0，
 * 就现分配一张清零的新页表挂上去。
 * 返回下一级页表的【物理地址】（不是虚拟地址）。 */
static uint64_t child_of(union PageMapEntry *table, int idx, int user) {
  union PageMapEntry *e = &table[idx];
  if (!e->bits.present) {
    uint64_t child_phys = pmm_alloc_zero(); /* 新页表页，内容全 0 = 空表 */
    e->data = 0;                            /* 先清掉整项的残留位 */
    e->bits.present = 1;                    /* 有效 */
    e->bits.writable = 1;                   /* 中间级表一律可写，权限留给末级页 */
    e->bits.user = (uint64_t)user;          /* 用户态映射要求整条链都置 U/S */
    entry_set_phys(e, child_phys);          /* 记录下一级表的物理页框号 */
    return child_phys;
  }
  return entry_phys(*e); /* 已存在：直接取回它的物理地址 */
}

void map_pages(uint64_t vaddr, uint64_t paddr_first, size_t pages,
               uint64_t flags) {
  const int user = (flags & P_USER) ? 1 : 0;

  /* 逐页处理：虚拟页和物理页一一对应，都按 4K 步进 */
  for (size_t i = 0; i < pages; i++) {
    uint64_t va = vaddr + i * 0x1000;
    uint64_t pa = paddr_first + i * 0x1000;

    union LinearAddress4Level addr;
    addr.value = va; /* 拆出各级表索引 */

    /* 从 PML4 往下走 PML4 -> PDPT -> PD，缺哪一级就补哪一级。
     * CR3 低 12 位是标志位，先与掉，只留 PML4 的物理地址。 */
    uint64_t table = get_cr3() & 0xFFFFFFFFFF000ULL; /* PML4 物理地址 */
    table = child_of(table_at(table), addr_part(addr, 4), user); /* PML4[47:39] */
    table = child_of(table_at(table), addr_part(addr, 3), user); /* PDPT[38:30] */
    table = child_of(table_at(table), addr_part(addr, 2), user); /* PD  [29:21] */

    /* 最后一级：PT 里挂真正指向数据页的表项 */
    union PageMapEntry *pt = table_at(table);
    union PageMapEntry *e = &pt[addr_part(addr, 1)]; /* PT[20:12] */
    e->data = 0;
    e->bits.present = 1;
    e->bits.writable = (flags & P_WRITE) ? 1 : 0; /* 读写权限由调用方 flags 决定 */
    e->bits.user = (uint64_t)user;
    entry_set_phys(e, pa); /* 写入目标物理页框号 */

    /* 该虚拟地址可能已被 TLB 缓存，改完映射后手动失效，避免用到旧表项 */
    asm volatile("invlpg (%0)" : : "r"(va));
  }
}

uint8_t *mmio_map(uint64_t phys_page, size_t pages) {
  static uint64_t next_vaddr = MMIO_VBASE; /* 每次从高半区往后发新地址，不复用 */
  uint8_t *v = (uint8_t *)next_vaddr;
  /* phys_page 低 4 位是 BAR 的标志位，先清掉再用作页物理地址 */
  map_pages(next_vaddr, phys_page & ~0xFULL, pages, P_PRESENT | P_WRITE);
  next_vaddr += pages * 0x1000; /* 预留连续虚拟地址，避免下次映射重叠 */
  return v;
}
