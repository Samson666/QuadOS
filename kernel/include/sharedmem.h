#pragma once

#include "types.h"
#include "config.h"

typedef struct {
    uint32_t size_in_pages;
    uint32_t* physical_pages;
    uint32_t owner_task_id; // 0 means kernel owned
} SharedMemory; // SharedMemoryObject?

typedef struct {
    uint32_t vaddr;
    uint32_t shmem_obj;
} SharedMemoryMapping; // MOVES AROUND, DONT STORE REFERENCES TO THIS DIRECTLY!

// TODO: dynamic array
typedef struct {
    SharedMemoryMapping mappings[MAX_SHARED_MEMORY_MAPPINGS];
    int num;
    uint32_t vaddr_start;
} SharedMemoryMappingPool;

extern SharedMemory shmem[MAX_SHARED_MEMORY_OBJS];

void sharedmem_init();
int32_t sharedmem_create(uint32_t size, uint32_t owner_task_id);
void sharedmem_destroy(int32_t id);
bool sharedmem_exists(int32_t id);
void* sharedmem_map(int32_t id, uint32_t task_id);
void sharedmem_unmap(int32_t id, uint32_t task_id);
void shmem_print_mappings(SharedMemoryMappingPool* pool);
