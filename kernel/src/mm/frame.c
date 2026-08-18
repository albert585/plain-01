/*
 * Ported from iNagisa/OS_design (https://github.com/iNagisa/OS_design)
 * Copyright (C) iNagisa. Licensed under 0BSD.
 * Adapted for plain-01.
 */

#include <mm/frame.h>
#include <mm/hhdm.h>
#include <limine.h>
#include <lib/memory.h>

__attribute__((used, section(".limine_requests"))) static volatile struct
    limine_memmap_request memmap_request = {
        .id = LIMINE_MEMMAP_REQUEST,
        .revision = 0,
};

frame_allocator pmm;

/* ---------- 位图基本操作 ---------- */

static void bitmap_set(frame_bitmap *bm, size_t index, bool value)
{
    size_t word = index / 8;
    size_t bit  = index % 8;
    if (value)
        bm->buffer[word] |= (uint8_t)(1u << bit);
    else
        bm->buffer[word] &= (uint8_t)~(1u << bit);
}

static bool bitmap_get(const frame_bitmap *bm, size_t index)
{
    return (bm->buffer[index / 8] >> (index % 8)) & 1;
}

static void bitmap_set_range(frame_bitmap *bm, size_t start, size_t end, bool value)
{
    for (size_t i = start; i < end; ++i)
        bitmap_set(bm, i, value);
}

/* 找一段连续 count 个 value 位，返回起始下标，失败返回 (size_t)-1 */
static size_t bitmap_find_range(const frame_bitmap *bm, size_t count, bool value)
{
    size_t run = 0, start = 0;
    for (size_t i = 0; i < bm->length; ++i)
    {
        if (bitmap_get(bm, i) == value)
        {
            if (run == 0)
                start = i;
            ++run;
            if (run >= count)
                return start;
        }
        else
        {
            run = 0;
        }
    }
    return (size_t)-1;
}

/* ---------- 初始化 ---------- */

void frame_init(void)
{
    struct limine_memmap_response *map = memmap_request.response;
    if (!map || map->entry_count == 0)
        return;

    /* 1. 用最高一块 usable 区域算物理内存上限 */
    uint64_t memory_size = 0;
    for (uint64_t i = 0; i < map->entry_count; ++i)
    {
        struct limine_memmap_entry *e = map->entries[i];
        if (e->type == LIMINE_MEMMAP_USABLE && e->base + e->length > memory_size)
            memory_size = e->base + e->length;
    }

    size_t total_frames = (size_t)(memory_size / PAGE_SIZE_4K);
    size_t bitmap_size  = (total_frames + 7) / 8;

    /* 2. 找一块放得下位图的最小 usable 区域（贪心） */
    uint64_t bitmap_phys = 0;
    uint64_t best_len   = (uint64_t)-1;
    for (uint64_t i = 0; i < map->entry_count; ++i)
    {
        struct limine_memmap_entry *e = map->entries[i];
        if (e->type == LIMINE_MEMMAP_USABLE &&
            e->length >= bitmap_size && e->length < best_len)
        {
            best_len   = e->length;
            bitmap_phys = e->base;
        }
    }

    if (!bitmap_phys)
        return; /* 理论上到不了这里 */

    /* 3. 全部先标成占用（0），再把 usable 区域标成空闲（1） */
    pmm.bitmap.buffer = (uint8_t *)phys_to_virt(bitmap_phys);
    pmm.bitmap.length = total_frames;
    kmemset(pmm.bitmap.buffer, 0, bitmap_size);

    pmm.total_frames = 0;
    for (uint64_t i = 0; i < map->entry_count; ++i)
    {
        struct limine_memmap_entry *e = map->entries[i];
        if (e->type == LIMINE_MEMMAP_USABLE)
        {
            size_t start = (size_t)(e->base / PAGE_SIZE_4K);
            size_t cnt   = (size_t)(e->length / PAGE_SIZE_4K);
            bitmap_set_range(&pmm.bitmap, start, start + cnt, true);
            pmm.total_frames += cnt;
        }
    }

    /* 4. 位图自己占的帧标记回占用 */
    size_t bm_start = (size_t)(bitmap_phys / PAGE_SIZE_4K);
    size_t bm_cnt   = (bitmap_size + PAGE_SIZE_4K - 1) / PAGE_SIZE_4K;
    bitmap_set_range(&pmm.bitmap, bm_start, bm_start + bm_cnt, false);

    pmm.free_frames = pmm.total_frames - bm_cnt;
}

/* ---------- 分配 / 释放 ---------- */

uint64_t frame_alloc(size_t count)
{
    size_t index = bitmap_find_range(&pmm.bitmap, count, true);
    if (index == (size_t)-1)
        return 0;

    bitmap_set_range(&pmm.bitmap, index, index + count, false);
    pmm.free_frames -= count;
    return index * PAGE_SIZE_4K;
}

void frame_free(uint64_t addr, size_t count)
{
    if (addr == 0)
        return;
    size_t index = (size_t)(addr / PAGE_SIZE_4K);
    bitmap_set_range(&pmm.bitmap, index, index + count, true);
    pmm.free_frames += count;
}
