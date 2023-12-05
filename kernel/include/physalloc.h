#pragma once

// physical memory manager

#include "types.h"

void pmm_init(uint32_t mem_low, uint32_t mem_high);
uint32_t pmm_alloc_pageframe();
void pmm_free_pageframe(uint32_t addr);
uint32_t pmm_get_total_allocated_pages();
