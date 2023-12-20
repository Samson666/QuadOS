// Title:	 		physalloc.h
// Description:
// First Created: 	19.12.2023
// Last Change:
 
#ifndef __PHYSALLOC_H
#define __PHYSALLOC_H
#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

void pmm_init(uint32_t mem_low, uint32_t mem_high);
uint32_t pmm_alloc_pageframe();
void pmm_free_pageframe(uint32_t addr);
uint32_t pmm_get_total_allocated_pages();

#ifdef __cplusplus
}
#endif
#endif //__PHYSALLOC_H

// physical memory manager


