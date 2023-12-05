#include "memory.h"

#include "util.h"
#include "interrupts.h"
#include "log.h"
#include "physalloc.h"
#include "kmalloc.h"
#include "multiboot.h"

// rename to paging.c ?

#define NUM_PAGE_DIRS 256
static uint32_t page_dirs[NUM_PAGE_DIRS][1024] __attribute__((aligned(4096)));
static uint8_t page_dir_used[NUM_PAGE_DIRS]; // todo: use bitfield

int mem_num_vpages;

static void enable_paging();
static void invalidate(int vaddr);
static void sync_page_dirs();

void init_memory(uint32_t mem_high) {
    mem_num_vpages = 0;

    // unmap the first 4 mb
    initial_page_dir[0] = 0;
    invalidate(0);

    // recursive table mapping
    initial_page_dir[1023] = ((uint32_t) initial_page_dir - KERNEL_START) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;
    invalidate(0xFFFFF000);

    pmm_init(0x100000 * 4, mem_high);

    memset(page_dirs, 0, 0x1000 * NUM_PAGE_DIRS);
    memset(page_dir_used, 0, NUM_PAGE_DIRS);
}

void mem_change_page_directory(uint32_t* pd) {
    assert_msg((uint32_t) pd > KERNEL_START, "whats up");
    pd = (uint32_t*) (((uint32_t) pd) - KERNEL_START); // calc the physical address
    asm volatile(
        "mov %0, %%eax \n"
        "mov %%eax, %%cr3 \n"
        :: "m"(pd)
    );
}

uint32_t* mem_get_current_page_directory() {
    uint32_t pd;
    asm volatile(
        "mov %%cr3, %0"
        : "=r"(pd)
    );
    pd += KERNEL_START; // calc the virtual address
    return (uint32_t*) pd;
}

void enable_paging() {
    asm volatile(
        "mov %cr4, %ebx \n"
        "or $0x10, %ebx \n"
        "mov %ebx, %cr4 \n"

        "mov %cr0, %ebx \n"
        "or $0x80000000, %ebx \n"
        "mov %ebx, %cr0 \n"
    );
}

void invalidate(int vaddr) {
    asm volatile("invlpg %0" :: "m"(vaddr));
}

// addresses need to be 4096 aligned
void mem_map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags) {
    VERIFY_INTERRUPTS_DISABLED;
    // kernel_log("pdir=%x mem_map_page: %x mapped to %x", mem_get_current_page_directory(), virt_addr, phys_addr);

    uint32_t* prev_page_dir = 0;
    if (virt_addr >= KERNEL_START) {
        // optimization: just copy from current pagedir to all others, including init_page_dir
        //              we might just wanna have init_page_dir be page_dirs[0] instead
        //              then we would have to build it in boot.asm in assembly
        
        // write to initial_page_dir, so that we can sync that across all others

        prev_page_dir = mem_get_current_page_directory();

        if (prev_page_dir != initial_page_dir)
            mem_change_page_directory(initial_page_dir);
    }

    // extract indices from the vaddr
    uint32_t pd_index = virt_addr >> 22;
    uint32_t pt_index = virt_addr >> 12 & 0x3FF;

    uint32_t* page_dir = REC_PAGEDIR;

    // page tables can only be directly accessed/modified using the recursive strat?
    // > yes since their physical page is not mapped into memory
    uint32_t* pt = REC_PAGETABLE(pd_index);

    if (!(page_dir[pd_index] & PAGE_FLAG_PRESENT)) {
        // allocate a page table
        uint32_t pt_paddr = pmm_alloc_pageframe();
        // kernel_log("creating table %x", pt_paddr);

        page_dir[pd_index] = pt_paddr | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_OWNER | flags;
        invalidate(virt_addr);

        // we can now access it directly using the recursive strategy
        for (uint32_t i = 0; i < 1024; i++) {
            pt[i] = 0;
        }
    }

    pt[pt_index] = phys_addr | PAGE_FLAG_PRESENT | flags;
    mem_num_vpages++;
    invalidate(virt_addr);

    if (prev_page_dir != 0) {
        // ... then sync that across all others
        sync_page_dirs();
        // we changed to init page dir, now we need to change back
        if (prev_page_dir != initial_page_dir)
            mem_change_page_directory(prev_page_dir);
    }
}

// returns page table entry (physical address and flags)
uint32_t mem_unmap_page(uint32_t virt_addr) {
    VERIFY_INTERRUPTS_DISABLED;

    uint32_t* prev_page_dir = 0;
    if (virt_addr >= KERNEL_START) {
        // optimization: just copy from current pagedir to all others, including init_page_dir
        //              we might just wanna have init_page_dir be page_dirs[0] instead
        //              then we would have to build it in boot.asm in assembly
        
        // write to initial_page_dir, so that we can sync that across all others

        // kernel_log("KERNEL SPACE !!!!!!!!!!");

        prev_page_dir = mem_get_current_page_directory();

        if (prev_page_dir != initial_page_dir)
            mem_change_page_directory(initial_page_dir);
    }

    uint32_t pd_index = virt_addr >> 22;
    uint32_t pt_index = virt_addr >> 12 & 0x3FF;

    uint32_t* page_dir = REC_PAGEDIR;

    uint32_t pd_entry = page_dir[pd_index];
    assert_msg(pd_entry & PAGE_FLAG_PRESENT, "tried to free page from a non present page table?");

    uint32_t* pt = REC_PAGETABLE(pd_index);

    uint32_t pte = pt[pt_index];
    assert_msg(pte & PAGE_FLAG_PRESENT, "tried to free non present page");

    pt[pt_index] = 0;

    mem_num_vpages--;

    bool remove = true;
    // optimization: keep track of the number of pages present in each page table
    for (uint32_t i = 0; i < 1024; i++) {
        if (pt[i] & PAGE_FLAG_PRESENT) {
            remove = false;
            break;
        }
    }

    if (remove) {
        // table is empty, destroy its physical frame if we own it.
        uint32_t pde = page_dir[pd_index];
        if (pde & PAGE_FLAG_OWNER) {
            // kernel_log("REMOVING PAGETABLE");
            uint32_t pt_paddr = P_PHYS_ADDR(pde);
            // kernel_log("removing page table %x", pt_paddr);
            pmm_free_pageframe(pt_paddr);
            page_dir[pd_index] = 0;
        }
    }

    invalidate(virt_addr);

    // free it here for now
    if (pte & PAGE_FLAG_OWNER) {
        pmm_free_pageframe(P_PHYS_ADDR(pte));
    }

    if (prev_page_dir != 0) {
        // ... then sync that across all others
        sync_page_dirs();
        // we changed to init page dir, now we need to change back
        if (prev_page_dir != initial_page_dir)
            mem_change_page_directory(prev_page_dir);
    }

    return pte;
}

uint32_t* mem_alloc_page_dir() {
    VERIFY_INTERRUPTS_DISABLED;

    for (int i = 0; i < NUM_PAGE_DIRS; i++) {
        if (!page_dir_used[i]) {
            page_dir_used[i] = true;

            uint32_t* page_dir = page_dirs[i];
            memset(page_dir, 0, 0x1000);

            // first 768 entries are user page tables
            for (int i = 0; i < 768; i++) {
                page_dir[i] = 0;
            }

            // next 256 are kernel (except last)
            for (int i = 768; i < 1023; i++) {
                page_dir[i] = initial_page_dir[i] & ~PAGE_FLAG_OWNER; // we don't own these though
            }

            // recursive mapping
            page_dir[1023] = (((uint32_t) page_dir) - KERNEL_START) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;
            return page_dir;
        }
    }

    assert_msg(false, "no page dirs left!");
    return 0;
}

void mem_free_page_dir(uint32_t* page_dir) {
    VERIFY_INTERRUPTS_DISABLED;

    uint32_t* prev_pagedir = mem_get_current_page_directory();
    mem_change_page_directory(page_dir);

    uint32_t pagedir_index = ((uint32_t)page_dir) - ((uint32_t) page_dirs);
    pagedir_index /= 4096;

    // kernel_log("pagedir_index = %u", pagedir_index);

    uint32_t* pd = REC_PAGEDIR;
    for (int i = 0; i < 768; i++) {
        int pde = pd[i];
        if (pde == 0)
            continue;
        
        uint32_t* ptable = REC_PAGETABLE(i);
        for (int j = 0; j < 1024; j++) {
            uint32_t pte = ptable[j];

            if (pte & PAGE_FLAG_OWNER) {
                pmm_free_pageframe(P_PHYS_ADDR(pte));
            }
        }
        memset(ptable, 0, 4096); // is this necessary?

        if (pde & PAGE_FLAG_OWNER) {
            // we created this pagetable, lets free it now
            pmm_free_pageframe(P_PHYS_ADDR(pde));
        }
        pd[i] = 0;
    }

    // do we need to clean the kernel portion?

    page_dir_used[pagedir_index] = 0;
    mem_change_page_directory(prev_pagedir);
}

// sync the kernel portions of all in-use pagedirs
void sync_page_dirs() {
    VERIFY_INTERRUPTS_DISABLED;

    for (int i = 0; i < NUM_PAGE_DIRS; i++) {
        if (page_dir_used[i]) {
            uint32_t* page_dir = page_dirs[i];
            
            for (int i = 768; i < 1023; i++) {
                page_dir[i] = initial_page_dir[i] & ~PAGE_FLAG_OWNER; // we don't own these though
            }
        }
    }
}

uint32_t mem_get_phys_from_virt(uint32_t virt_addr) {
    VERIFY_INTERRUPTS_DISABLED;

    uint32_t pd_index = virt_addr >> 22;
    uint32_t* page_dir = REC_PAGEDIR;
    if (!(page_dir[pd_index] & PAGE_FLAG_PRESENT)) {
        return -1;
    }

    uint32_t pt_index = virt_addr >> 12 & 0x3FF;
    uint32_t* pt = REC_PAGETABLE(pd_index);

    return P_PHYS_ADDR(pt[pt_index]);
}

bool mem_is_valid_vaddr(uint32_t vaddr) {
    VERIFY_INTERRUPTS_DISABLED;
    
    uint32_t pd_index = vaddr >> 22;
    uint32_t pt_index = vaddr >> 12 & 0x3FF;

    uint32_t* page_dir = REC_PAGEDIR;

    uint32_t pd_entry = page_dir[pd_index];
    if (!(pd_entry & PAGE_FLAG_PRESENT))
        return false;
    
    uint32_t* pt = REC_PAGETABLE(pd_index);

    uint32_t pt_entry = pt[pt_index];
    return pt_entry & PAGE_FLAG_PRESENT;
}
