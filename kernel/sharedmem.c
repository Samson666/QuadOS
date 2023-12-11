#include "sharedmem.h"

#include "util.h"
#include "kmalloc.h"
#include "memory.h"
#include "physalloc.h"
#include "log.h"
#include "tasks.h"
#include "interrupts.h"

SharedMemory shmem[MAX_SHARED_MEMORY_OBJS];
SharedMemoryMappingPool kernel_shmem_mappings;

static int32_t find_available_shmem_slot();

void sharedmem_init() {
    memset(shmem, 0, sizeof(shmem));
    memset(&kernel_shmem_mappings, 0, sizeof(kernel_shmem_mappings));
    kernel_shmem_mappings.vaddr_start = KERNEL_SHARED_MEMORY;
}

// Functionname 	: sharedmem_create
// Parameters		: size, id of the owner task
// Returns			: id of the sharedmem object
// Description		: Allocates memory for shared memory
// Note				: problem: how do two processes agree on the same shmem obj id?

int32_t sharedmem_create(uint32_t size, uint32_t owner_task_id) {
    kernel_log("Function sharedmem_create size: %d owner_task_id: %d", size, owner_task_id);
    //Are interrupts disabled? If not assert!
    VERIFY_INTERRUPTS_DISABLED;
    assert(size);

    int32_t id = find_available_shmem_slot();
    SharedMemory* obj = &shmem[id];

    uint32_t num_pages = CEIL_DIV(size, 0x1000);
    obj->size_in_pages = num_pages;
    obj->physical_pages = kmalloc(num_pages * sizeof(uint32_t));
    obj->owner_task_id = owner_task_id;

    for (int i = 0; i < num_pages; i++) {
        obj->physical_pages[i] = pmm_alloc_pageframe();
    }

    return id;
}

// Functionname 	: sharedmem_resize
// Parameters		: id of the sharedmem object
// Returns			: void
// Description		: resizes the allocated memory of the shared memory object
// Note				: mainly used for resizing windows

void sharedmem_resize(int32_t id, uint32_t newsize)
{
    int i = 0;
    //Are interrupts disabled? If not assert!
    //VERIFY_INTERRUPTS_DISABLED;
    //assert(newsize);
    
    SharedMemory* obj = &shmem[id];
    uint32_t num_pages = CEIL_DIV(newsize, 0x1000);

    kernel_log("size in bytes before resizing: %d", obj->size_in_pages * 4096);
    
    for (i = 0; i < obj->size_in_pages; i++) {
        pmm_free_pageframe(obj->physical_pages[i]);
    }
    kfree(obj->physical_pages);

    obj->size_in_pages = num_pages;
    obj->physical_pages = kmalloc(num_pages * sizeof(uint32_t));

     for (i = 0; i < num_pages; i++) {
        obj->physical_pages[i] = pmm_alloc_pageframe();
    }

    kernel_log("size in bytes after resizing: %d", obj->size_in_pages * 4096);
}


void sharedmem_destroy(int32_t id) {
    VERIFY_INTERRUPTS_DISABLED;
    assert(sharedmem_exists(id));

    SharedMemory* obj = &shmem[id];
    for (int t = 0; t < MAX_TASKS; t++) {
        Task* task = &tasks[t];
        if (task->state == TASK_STATE_DEAD)
            continue;
        
        SharedMemoryMappingPool* pool = &task->shmem;

        for (int i = 0; i < pool->num; i++) {
            if (pool->mappings[i].shmem_obj == id) {
                sharedmem_unmap(id, task->id);
            }
        }
    }

    SharedMemoryMappingPool* pool = &kernel_shmem_mappings;
    for (int i = 0; i < pool->num; i++) {
        if (pool->mappings[i].shmem_obj == id) {
            sharedmem_unmap(id, 0);
        }
    }

    for (int i = 0; i < obj->size_in_pages; i++) {
        pmm_free_pageframe(obj->physical_pages[i]);
    }

    kfree(obj->physical_pages);
    memset(obj, 0, sizeof(SharedMemory));
}

bool sharedmem_exists(int32_t id) {
    if (id < 0 || id >= MAX_SHARED_MEMORY_OBJS)
        return false;
    return shmem[id].size_in_pages != 0;
}

// Functionname 	: insert_into_pool
// Parameters		: 
// Returns			: 
// Description		: 
// Note				: 
 
// dont store pointer returned by this
static SharedMemoryMapping* insert_into_pool(SharedMemoryMappingPool* pool, int size_in_pages, int shmem_id) {
    assert(pool->num < MAX_SHARED_MEMORY_MAPPINGS);

    SharedMemoryMapping* mapping = pool->mappings + pool->num;
    mapping->shmem_obj = shmem_id;
    mapping->vaddr = pool->vaddr_start;

    // TODO: search for gaps and insert it there

    if (pool->num > 0) {
        SharedMemoryMapping* prev = &pool->mappings[pool->num - 1];
        int prev_size = shmem[prev->shmem_obj].size_in_pages;
        mapping->vaddr = prev->vaddr + prev_size * 0x1000;
    }

    pool->num++;
    return mapping;
}

static void remove_from_pool(SharedMemoryMappingPool* pool, int index) {
    assert(index < pool->num);
    for (int i = index; i < pool->num - 1; i++) {
        pool->mappings[i] = pool->mappings[i + 1];
    }
    pool->num--;
}

// Functionname 	: sharedmem_map
// Parameters		: id of shared memory object
// Returns			: ptr to virtual adress of sharemem object
// Description		: maps the sharedmem object to virtual memory and given task
// Note				: 
 
void* sharedmem_map(int32_t id, uint32_t task_id) {
    VERIFY_INTERRUPTS_DISABLED;
    assert(sharedmem_exists(id));
    kernel_log("sharedmem_map    shmem obj=%u task_id=%u", id, task_id);

    SharedMemory* obj = &shmem[id];
    SharedMemoryMappingPool* pool = &kernel_shmem_mappings;
    if (task_id != 0) {
        Task* task = get_task(task_id);
        pool = &task->shmem;
    }
    assert(pool);

    SharedMemoryMapping* mapping = insert_into_pool(pool, obj->size_in_pages, id);
    assert(mapping);

    bool map_to_kernel = task_id == 0;

    uint32_t* prev_pd = NULL;
    if (!map_to_kernel) {
        prev_pd = mem_get_current_page_directory();
        Task* task = get_task(task_id);
        mem_change_page_directory(task->pagedir);
    }

    for (int i = 0; i < obj->size_in_pages; i++) {
        uint32_t flags = PAGE_FLAG_WRITE;
        if (!map_to_kernel)
            flags |= PAGE_FLAG_USER;

        //kernel_log("sharedmem: mapping %x to %x", mapping->vaddr + i * 0x1000, obj->physical_pages[i]);
        mem_map_page(mapping->vaddr + i * 0x1000, obj->physical_pages[i], flags);
    }

    if (!map_to_kernel) {
        mem_change_page_directory(prev_pd);
    }

    assert(mapping->vaddr);
    return mapping->vaddr;
}

void sharedmem_unmap(int32_t id, uint32_t task_id) {
    VERIFY_INTERRUPTS_DISABLED;
    assert(sharedmem_exists(id));
    // kernel_log("sharedmem_unmap    shmem obj=%u task_id=%u", id, task_id);

    SharedMemory* obj = &shmem[id];
    SharedMemoryMappingPool* pool = &kernel_shmem_mappings;
    if (task_id != 0) {
        Task* task = get_task(task_id);
        pool = &task->shmem;
    }

    SharedMemoryMapping* mapping = NULL;
    int pool_index = -1;
    for (int i = 0; i < pool->num; i++) {
        if (pool->mappings[i].shmem_obj == id) {
            mapping = &pool->mappings[i];
            pool_index = i;
            break;
        }
    }

    assert(mapping);
    // assert_msg(slot != -1, "tried to unmap shmem when its not mapped!");

    bool kernel_space = task_id == 0;

    uint32_t* prev_pd = NULL;
    if (!kernel_space) {
        prev_pd = mem_get_current_page_directory();

        Task* task = get_task(task_id);
        mem_change_page_directory(task->pagedir);
    }

    for (int i = 0; i < obj->size_in_pages; i++) {
        mem_unmap_page(mapping->vaddr + 0x1000 * i);
    }

    if (!kernel_space) {
        mem_change_page_directory(prev_pd);
    }

    remove_from_pool(pool, pool_index);
}

static int32_t find_available_shmem_slot() {
    for (int i = 0; i < MAX_SHARED_MEMORY_OBJS; i++) {
        if (!sharedmem_exists(i)) {
            return i;
        }
    }
    assert_msg(0, "no more shmem slots!");
}

void shmem_print_mappings(SharedMemoryMappingPool* pool) {
    if (pool == NULL)
        pool = &kernel_shmem_mappings;
    
    for (int i = 0; i < pool->num; i++) {
        SharedMemoryMapping* mapping = &pool->mappings[i];
        SharedMemory* obj = &shmem[mapping->shmem_obj];
        
        kernel_log("  [%u] vaddr=%x size=%x maps_to=%u", i, mapping->vaddr, obj->size_in_pages * 0x1000, mapping->shmem_obj);
    }
}
