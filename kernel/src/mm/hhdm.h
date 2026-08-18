/*
 * Ported from iNagisa/OS_design (https://github.com/iNagisa/OS_design)
 * Copyright (C) iNagisa. Licensed under 0BSD.
 * Adapted for plain-01.
 */

#pragma once

#include <stdint.h>

/*
 * HHDM（Higher Half Direct Map）
 * Limine 提供的"物理内存 -> 高半区虚拟地址"直射偏移。
 * 没有 page table 操作前，这是访问物理内存的唯一通道。
 */

extern uint64_t hhdm_offset;

void hhdm_init(void);

#define phys_to_virt(addr) ((uint64_t)(addr) + hhdm_offset)
#define virt_to_phys(addr) ((uint64_t)(addr) - hhdm_offset)
