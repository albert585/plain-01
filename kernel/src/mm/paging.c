/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Derived from MikanOS kernel/paging.cpp (Apache-2.0), Copyright (c)
 * 2018-2022 Kota Uchida.  This translation implements the multi-page
 * recursive page-table walk of MikanOS' SetupPageMaps, adapted to
 * Limine's HHDM (page-table pages are accessed as phys + hhdm_offset)
 * and to the plain-01 bump page allocator.  See the LICENSE file in this
 * directory for the Apache License 2.0 text.
 */

#include "mm/paging.h"

#include <stddef.h>
#include <stdint.h>

#include "mm/mem.h" /* pmm_alloc_zero, get_hhdm_base, get_cr3, P_* */

/* Convert a PHYSICAL page-table address to the virtual address Limine
 * mapped it at (HHDM: virt = phys + offset). */
static union PageMapEntry *table_at(uint64_t phys) {
  return (union PageMapEntry *)(phys + get_hhdm_base());
}

/* Ensure table[idx] points at an existing next-level table; allocate a
 * zeroed page on demand.  Returns the next-level table's PHYSICAL address. */
static uint64_t child_of(union PageMapEntry *table, int idx, int user) {
  union PageMapEntry *e = &table[idx];
  if (!e->bits.present) {
    uint64_t child_phys = pmm_alloc_zero();
    e->data = 0;
    e->bits.present = 1;
    e->bits.writable = 1;
    e->bits.user = (uint64_t)user;
    entry_set_phys(e, child_phys);
    return child_phys;
  }
  return entry_phys(*e);
}

void map_pages(uint64_t vaddr, uint64_t paddr_first, size_t pages,
               uint64_t flags) {
  const int user = (flags & P_USER) ? 1 : 0;

  for (size_t i = 0; i < pages; i++) {
    uint64_t va = vaddr + i * 0x1000;
    uint64_t pa = paddr_first + i * 0x1000;

    union LinearAddress4Level addr;
    addr.value = va;

    /* Walk PML4 -> PDPT -> PD, allocating missing tables. */    uint64_t table = get_cr3() & 0xFFFFFFFFFF000ULL; /* PML4 physical */
    table = child_of(table_at(table), addr_part(addr, 4), user);
    table = child_of(table_at(table), addr_part(addr, 3), user);
    table = child_of(table_at(table), addr_part(addr, 2), user);

    /* Last level: the actual data-page entry. */
    union PageMapEntry *pt = table_at(table);
    union PageMapEntry *e = &pt[addr_part(addr, 1)];
    e->data = 0;
    e->bits.present = 1;
    e->bits.writable = (flags & P_WRITE) ? 1 : 0;
    e->bits.user = (uint64_t)user;
    entry_set_phys(e, pa);

    asm volatile("invlpg (%0)" : : "r"(va));
  }
}

uint8_t *mmio_map(uint64_t phys_page, size_t pages) {
  static uint64_t next_vaddr = MMIO_VBASE; /* hand out fresh high-half pages */
  uint8_t *v = (uint8_t *)next_vaddr;
  map_pages(next_vaddr, phys_page & ~0xFULL, pages, P_PRESENT | P_WRITE);
  next_vaddr += pages * 0x1000;
  return v;
}
