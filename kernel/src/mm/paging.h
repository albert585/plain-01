/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Portions of this file are derived from the MikanOS kernel
 * (https://github.com/uchan-nos/mikanos), specifically the file
 * kernel/paging.hpp, which is:
 *      Copyright (c) 2018-2022 Kota Uchida
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 * Modifications made for the plain-01 kernel by Albert Chen:
 *  - translated from C++ to C (the union+bitfield types kept, the member
 *    functions replaced with free inline helpers),
 *  - adapted to Limine's HHDM: intermediate page-table pages are accessed
 *    via (phys + hhdm_offset) since virtual != physical under Limine,
 *  - adapted to the plain-01 bump page allocator (pmm_alloc_zero).
 *
 * This translation is distributed under the same Apache License 2.0.
 * See the file "LICENSE" in this directory for the full license text.
 */

#ifndef PLAIN_MM_PAGING_H
#define PLAIN_MM_PAGING_H

#include <stddef.h>
#include <stdint.h>

#include "mm/mem.h" /* P_PRESENT, P_WRITE, P_USER, MMIO_VBASE,
                        pmm_alloc_zero, get_hhdm_base, get_cr3 */

/* ---- Derived from MikanOS kernel/paging.hpp (Apache-2.0) ---- */

/* A canonical 4-level virtual address, split into the 5 fields used to
 * index PML4 / PDPT / PD / PT / offset. */
union LinearAddress4Level {
  uint64_t value;

  struct {
    uint64_t offset : 12;
    uint64_t page : 9;
    uint64_t dir : 9;
    uint64_t pdp : 9;
    uint64_t pml4 : 9;
    uint64_t : 16;
  } __attribute__((packed)) parts;
};

/* A single 64-bit page-table entry. */
union PageMapEntry {
  uint64_t data;

  struct {
    uint64_t present : 1;
    uint64_t writable : 1;
    uint64_t user : 1;
    uint64_t write_through : 1;
    uint64_t cache_disable : 1;
    uint64_t accessed : 1;
    uint64_t dirty : 1;
    uint64_t huge_page : 1;
    uint64_t global : 1;
    uint64_t : 3;

    uint64_t addr : 40;
    uint64_t : 12;
  } __attribute__((packed)) bits;
};

/* C replacements for MikanOS' LinearAddress4Level::Part / SetPart. */
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

/* C replacements for MikanOS' PageMapEntry::Pointer / SetPointer.
 * NOTE: here the stored value is a PHYSICAL address (Limine's HHDM is
 * applied only when dereferencing, not when storing). */
static inline uint64_t entry_phys(const union PageMapEntry e) {
  return e.bits.addr << 12;
}

static inline void entry_set_phys(union PageMapEntry *e, uint64_t phys) {
  e->bits.addr = phys >> 12;
}

/* Map `pages` consecutive 4K physical pages starting at paddr_first onto
 * consecutive 4K virtual pages starting at vaddr.  Intermediate page tables
 * are allocated on demand via pmm_alloc_zero() and accessed through HHDM. */
void map_pages(uint64_t vaddr, uint64_t paddr_first, size_t pages,
               uint64_t flags);

/* Map a device BAR of `pages` pages to a fresh high-half address and return
 * the virtual base.  `pages` covers the whole BAR (e.g. xHCI 16K => 4). */
uint8_t *mmio_map(uint64_t phys_page, size_t pages);

#endif /* PLAIN_MM_PAGING_H */
