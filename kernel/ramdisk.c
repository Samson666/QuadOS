#include "ramdisk.h"

#include "fatfs/fatfs_diskio.h"
#include "fatfs/fatfs_ff.h"
#include "interrupts.h"
#include "util.h"
#include "types.h"
#include "time.h"
#include "rtc.h"
#include "log.h"

static struct {
    uint32_t location;
    uint32_t size;
    FATFS fs;
} ramdisk, ramdisk2;

void init_ramdisk(uint32_t location, uint32_t size) {
    kernel_log("Ram disk located at %x with size %u bytes", location, size);
    ramdisk.location = location;
    ramdisk.size = size;

    FRESULT res;

    res = f_mount(&ramdisk.fs, "0:", 0);

    if (res != FR_OK) {
        kernel_log("f_mount error: %u\n", (uint32_t) res);
        crash_and_burn();
    }
}

void init_ramdisk2(uint32_t location, uint32_t size) {
    kernel_log("Ram disk located at %x with size %u bytes", location, size);
    ramdisk2.location = location;
    ramdisk2.size = size;

    FRESULT res;

    res = f_mount(&ramdisk2.fs, "1:", 0);

    if (res != FR_OK) {
        kernel_log("f_mount error: %u\n", (uint32_t) res);
        crash_and_burn();
    }
}
DSTATUS disk_initialize(BYTE pdrv) {
    return 0;
}

DSTATUS disk_status(BYTE pdrv) {
    return 0;
}

DRESULT disk_read(BYTE pdrv, BYTE* buffer, DWORD sector, UINT count) {
    uint32_t offset = sector * RAMDISK_BLOCKSIZE;
    uint32_t size = count * RAMDISK_BLOCKSIZE;

    // disable_interrupts();
    memcpy(buffer, (uint8_t*) (ramdisk.location + offset), size);
    // enable_interrupts();

    return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buffer, DWORD sector, UINT count) {
    uint32_t offset = sector * RAMDISK_BLOCKSIZE;
    uint32_t size = count * RAMDISK_BLOCKSIZE;

    memcpy((uint8_t*) (ramdisk.location + offset), buffer, size);

    return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
    DRESULT dr = RES_ERROR;

    switch (cmd)
    {
    case CTRL_SYNC:
        dr = RES_OK;
        break;
    case GET_SECTOR_COUNT:
        *(DWORD*) buff = ramdisk.size / RAMDISK_BLOCKSIZE;
        dr = RES_OK;
        // printkf("disk_ioctl GET_SECTOR_COUNT: %d\n", value);
        break;
    case GET_BLOCK_SIZE:
        *(DWORD*) buff = RAMDISK_BLOCKSIZE;
        dr = RES_OK;
        break;
    }

    return dr;
}

DWORD get_fattime (void)
{
    struct tm stm;

    return (DWORD)(stm.tm_year - 80) << 25 |
           (DWORD)(stm.tm_mon + 1) << 21 |
           (DWORD)stm.tm_mday << 16 |
           (DWORD)stm.tm_hour << 11 |
           (DWORD)stm.tm_min << 5 |
           (DWORD)stm.tm_sec >> 1;
}
