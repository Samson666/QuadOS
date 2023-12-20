#pragma once

#include "types.h"
#include "ramdisk.h"
#include "../fatfs/fatfs_ff.h"
#include "slib.h"
#include "config.h"
#include "sharedmem.h"

// used for newly created tasks
typedef struct {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebx;
    uint32_t ebp;
    uint32_t eip;
} TaskReturnContext;

enum {
    TASK_STATE_DEAD,
    TASK_STATE_READY,
    TASK_STATE_IDLE,
    TASK_STATE_WAIT_FOR_EVENT,
    TASK_STATE_WAIT_FOR_REDRAW,
};

typedef struct {
    uint32_t interval;
    uint64_t next_fire;
    bool active;
} Timer;

typedef struct {
    uint32_t id;

    // kernel stack
    uint32_t kesp; // exchanged on task switch
    uint32_t kesp0; // top of kernel stack (highest address) ...
    // ... stack ptr is set to this (via TSS) when transitioning from user to kernel mode (on interrupt/syscall)

    uint32_t* pagedir; // this task's page directory, kernel tasks use the initial pagedir
    uint32_t is_kernel_task;
    uint32_t state; // task state enum
    FIL open_files[MAX_OPEN_FILES]; // hack
    void* event_buffer; // address to shared memory in kernel vmem (kernel-side)
    int32_t event_shmem_id; // used to share the buffer with userspace
    SharedMemoryMappingPool shmem;

    uint32_t heap_start; // page aligned
    uint32_t heap_end;

    DIR open_dir;

    Timer timers[MAX_TIMERS];
    OpenSharedLibrary slibs[MAX_SHARED_LIBS_PER_TASK];
    char* name[64];
} Task;

extern Task tasks[MAX_TASKS];
extern Task* current_task;
extern int num_tasks;

void setup_tasks();
int32_t create_user_task(const char* path);
void create_kernel_task(void* func);
void kill_task(uint32_t id);
void task_schedule();
Task* get_task(int id);
void create_named_kernel_task(void* func, char* name);
