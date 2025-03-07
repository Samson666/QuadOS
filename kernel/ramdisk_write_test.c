#pragma once

#include "ramdisk.h"
#include "kmalloc.h"
#include "string.h"

int32_t write_to_ramdisk(void)
{
    FIL file;
    FRESULT res;
    res = f_open(&file, "1:test.txt", FA_OPEN_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        kernel_log("ramdisk write test: failed to open file %s. error=%u", "test.txt", res);
        return -1;
    }

    char text[]="Bla Blaub test rhabarbera sldfkjiowerio sfjskldfjskl sdfklskdfj slkdfslk sldkfsld slkdfjslkdjf sldkfjsdklf";
    UINT btw = strlen(text);
    UINT bw;

    void *b = kmalloc(btw);
    memcpy(b,text,btw);
    kernel_log("before f_write");

    res = f_write(&file, b, btw, &bw);

    kernel_log("after f_write");
    if (res != FR_OK) {
        kernel_log("ramdisk write test: failed to write file %s. error=%u", "test.txt", res);
        return -1;
    }

    res = f_close(&file);
    if (res != FR_OK) {
        kernel_log("ramdisk write test: failed to close file %s. error=%u", "test.txt", res);
        return -1;
    }

    memset(b,0,btw);

    res = f_open(&file, "1:test.txt", FA_READ);
    if (res != FR_OK) {
        kernel_log("ramdisk write test: failed to open file %s. error=%u", "test.txt", res);
        return -1;
    }

    kernel_log("ramsidk write test: after open file to read");

    UINT br;
    void *wb = kmalloc(btw+2);
    f_read(&file, wb, btw, &br);
    if (res != FR_OK) {
        kernel_log("ramdisk write test: failed to read file %s. error=%u", "test.txt", res);
        return -1;
    }

    memset(wb+btw+1,NULL,1);
    kernel_log("%s", wb);

    res = f_close(&file);
    if (res != FR_OK) {
        kernel_log("ramdisk write test: failed to close file %s. error=%u", "test.txt", res);
        return -1;
    }

}