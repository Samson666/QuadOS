#include "os.h"

// copied from kmalloc.c

#include "syscalls.h"
#include "os_stdio.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef uint8_t  uint8_t;
typedef uint16_t uint16_t;
typedef uint32_t uint32_t;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t int32_t;
typedef int64_t s64;

#define ALIGNMENT 16

typedef struct {
	uint32_t size : 31; // including this header
	uint32_t used : 1;
#if ALIGNMENT > 4
	uint32_t pad[(ALIGNMENT / 4) - 1];
#endif
} ChunkHeader;

// last chunk has to be 0 to know when to stop

static uint32_t heap_start;
static uint32_t heap_size;
static uint32_t threshold;
static bool malloc_initialized = false;

static void merge_free_chunks();
static void change_heap_size(int new_size);

void malloc_init(unsigned int initial_heap_size) {
	heap_start = syscall_get_heap_start();
	//os_printf("init user malloc at %x", heap_start);
	heap_size = 0;
    threshold = 0;
	malloc_initialized = true;

    change_heap_size(initial_heap_size);

	//memset(pool_start, 0, pool_size);
	*((uint32_t*)heap_start) = 0;
}

static void* user_malloc(uint32_t bytes) {
	// debug_printf("os_malloc %x", bytes);
	uint32_t real_size = bytes + sizeof(ChunkHeader);
	if (real_size & (ALIGNMENT - 1)) {
		real_size += ALIGNMENT - (real_size & (ALIGNMENT - 1));
	}

	uint32_t pos = heap_start;
	while (true) {
		if (pos + real_size + sizeof(ChunkHeader) > heap_start + heap_size) {
            // download more ram
			// the +sizeof(ChunkHeader) is for the last stop chunk
            change_heap_size(pos + real_size + sizeof(ChunkHeader) - heap_start); // +1 ?
		}

		ChunkHeader* chunk = (ChunkHeader*) pos;

		if (!chunk->used && (chunk->size >= real_size || chunk->size == 0)) {
			// use this
			chunk->used = 1;

			if (chunk->size == 0) {
				// last chunk
				chunk->size = real_size;

				// make sure last chunk is 0
				ChunkHeader* last = (ChunkHeader*)(pos + real_size);
				last->size = 0;
				last->used = 0;
			} else {
				int32_t diff = (int32_t)chunk->size - (int32_t)real_size;

				if (diff >= 8) { // big enough for 1 4byte malloc
					// if big enough, split empty space into separate
					ChunkHeader* empty = (ChunkHeader*)(pos + real_size);
					empty->size = diff;
					empty->used = 0;

					chunk->size = real_size;
				} else {
					// use up the whole thing
				}
			}

			uint32_t addr = pos + sizeof(ChunkHeader);
			if (addr > threshold)
				threshold = addr;
			return (void*) addr;
		}

		pos += chunk->size;
	}

    assert_msg(false, "something wen't wrong in user malloc");
	return 0;
}

static void user_free(void* addr) {
	ChunkHeader* chunk = (ChunkHeader*) (((uint32_t) addr) - sizeof(ChunkHeader));

	if (!chunk->used) {
		return;
	}
	chunk->used = 0;

	// check if any/both adjacent chunks are empty
	//     !! cant do this, no way of going backwards
	// guess we'll just have to loop through the whole thing..
	merge_free_chunks();
}

void merge_free_chunks() {
	uint32_t pos = heap_start;
	while (true) {
		ChunkHeader* chunk = (ChunkHeader*)pos;
		if (chunk->size == 0)
			break;

		if (!chunk->used) {
			uint32_t next_pos = ((uint32_t)chunk) + chunk->size;
			while (true) {
				ChunkHeader* next = (ChunkHeader*) next_pos;
				// swallow it.
				if (next->used)
					break;

				if (next->size == 0) {
					// we've reached the end, stop merging
					chunk->size = 0;
					threshold = (uint32_t)chunk;
					return;
				}
				chunk->size += next->size;
				next_pos += next->size;
				//next->size = 69; // debug to show swallowed chunks, can't do this if we want to fill with debug
			}
		}

		pos += chunk->size;
		if (pos >= heap_start + heap_size) {
			break;
		}
	}
}

uint32_t get_total_bytes() {
	return threshold - heap_start;
}

void change_heap_size(int new_size) {
    // syscall_print("api: change_heap_size");
	syscall_set_heap_end(heap_start + new_size);
    heap_size = new_size;
}

void* os_malloc(uint32_t size) {
	return user_malloc(size);
}

void os_free(void* addr) {
	user_free(addr);
}
