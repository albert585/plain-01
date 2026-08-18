/*
 * Ported from iNagisa/OS_design (https://github.com/iNagisa/OS_design)
 * Copyright (C) iNagisa. Licensed under 0BSD.
 * Adapted for plain-01.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * 物理帧位图分配器（从 iNagisa/OS_design 移植，简化）
 *
 * 约定：bit = 1  表示该物理帧空闲，bit = 0 表示已占用。
 * （注意：这是"反直觉"的方向，alloc 找的是 1 的区间）
 */

#define PAGE_SIZE_4K 4096

typedef struct
{
    uint8_t *buffer;  /* 位图缓冲区（位于 HHDM 映射的物理内存中） */
    size_t   length;  /* 总位数 = 总帧数 */
} frame_bitmap;

typedef struct
{
    frame_bitmap bitmap;
    size_t       total_frames;
    size_t       free_frames;
} frame_allocator;

extern frame_allocator pmm;

void frame_init(void);

/* 返回物理地址，失败返回 0 */
uint64_t frame_alloc(size_t count);

/* addr 是物理地址，count 是帧数（不是字节数！） */
void frame_free(uint64_t addr, size_t count);
