#include "physalloc.h"

#include "util.h"
#include "log.h"

#define NUM_PAGE_FRAMES (0x100000000 / 0x1000 / 8) // note: reduced size
#define PAGE_FRAME_SIZE 0x1000

uint8_t physical_memory_bitmap[NUM_PAGE_FRAMES / 8]; // todo: dynamically allocate based on available RAM

static bool is_pf_used(uint32_t pf_index);
static void set_pf_used(uint32_t pf_index, bool used);

static uint32_t page_frame_min;
static uint32_t page_frame_max;
static uint32_t total_allocated;

void pmm_init(uint32_t mem_low, uint32_t mem_high) {
	page_frame_min = CEIL_DIV(mem_low, 0x1000);
	page_frame_max = mem_high / 0x1000;
	total_allocated = 0;

	memset(physical_memory_bitmap, 0, sizeof(physical_memory_bitmap));
}

uint32_t pmm_alloc_pageframe() {
	// fixme: properly handle min and max being misaligned to 8 pages
	// (if they don't start on a bitmap byte)

	uint32_t start = page_frame_min / 8 + ((page_frame_min & 7) != 0 ? 1 : 0);
	uint32_t end = page_frame_max / 8 - ((page_frame_max & 7) != 0 ? 1 : 0);

	for (uint32_t b = start; b < end; b++) {
		uint8_t byte = physical_memory_bitmap[b];
		if (byte == 0xFF)
			continue;

		for (uint32_t i = 0; i < 8; i++) {
			bool used = byte >> i & 1;

			if (!used) {
				byte ^= (-1 ^ byte) & (1 << i);
				physical_memory_bitmap[b] = byte;
				total_allocated++;
				uint32_t addr = (b * 8 + i) * PAGE_FRAME_SIZE;
				// kernel_log("alloc ppage %x", addr);
				return addr;
			}
		}
	}

	assert_msg(false, "OUT OF MEMORY");
	return 0;
}

void pmm_free_pageframe(uint32_t addr) {
	// kernel_log("free ppage %x", addr);
	uint32_t pf_index = addr / PAGE_FRAME_SIZE;

	assert_msg(is_pf_used(pf_index), "can't free; page is already not in use");

	set_pf_used(pf_index, 0);
	total_allocated--;
}

bool is_pf_used(uint32_t pf_index) {
	uint8_t byte = physical_memory_bitmap[pf_index >> 3];
	return byte >> (pf_index & 7) & 1;
}

void set_pf_used(uint32_t pf_index, bool used) {
	uint8_t byte = physical_memory_bitmap[pf_index >> 3];
	byte ^= (-used ^ byte) & (1 << (pf_index & 7));

	physical_memory_bitmap[pf_index >> 3] = byte;
}

uint32_t pmm_get_total_allocated_pages() {
	return total_allocated;
}
