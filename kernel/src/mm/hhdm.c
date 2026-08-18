/*
 * Ported from iNagisa/OS_design (https://github.com/iNagisa/OS_design)
 * Copyright (C) iNagisa. Licensed under 0BSD.
 * Adapted for plain-01.
 */

#include <mm/hhdm.h>
#include <limine.h>

__attribute__((used, section(".limine_requests"))) static volatile struct
    limine_hhdm_request hhdm_request = {
        .id = LIMINE_HHDM_REQUEST,
        .revision = 0,
};

uint64_t hhdm_offset = 0;

void hhdm_init(void)
{
    hhdm_offset = hhdm_request.response->offset;
}
