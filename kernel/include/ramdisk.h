#pragma once

#include "types.h"
#include "../fatfs/fatfs_ff.h"

#define RAMDISK_BLOCKSIZE 512

void init_ramdisk(uint32_t location, uint32_t size);
