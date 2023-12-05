#pragma once

#include "types.h"

void kmalloc_init(uint32_t heap_size);
void* kmalloc(uint32_t bytes);
void kfree(void* addr);
uint32_t kmalloc_get_total_bytes();
