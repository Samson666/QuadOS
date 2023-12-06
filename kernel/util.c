#include "util.h"

#include "interrupts.h"

#include "log.h"

#include "gui.h"
#include "graphics.h"

void* memset(void* vdest, uint8_t val, uint32_t len) {
    uint8_t* dest = (uint8_t*) vdest;
    uint8_t* temp = (uint8_t*) dest;
    for (; len != 0; len--) *temp++ = val;
    return dest;
}

void* memcpy(void* dest, const void* src, uint32_t len) {
    const uint8_t* sp = (const uint8_t*) src;
    uint8_t* dp = (uint8_t*) dest;
    for (; len != 0; len--) *dp++ = *sp++;
    return dest;
}

void* memcpy_asm(void* dest, const void* src, size_t bytes)
{
    size_t dwords = bytes/4;
    bytes %= 4;
    __asm__ volatile("cld\n" "rep movsl" : : "S" (src), "D" (dest), "c" (dwords));
    __asm__ volatile(        "rep movsb" : : "c" (bytes));
    return(dest);
}

uint32_t strcmp(const char* s1, const char* s2) {
    while (*s1 != '\0' && *s2 != '\0' && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

uint32_t strncmp(const char* s1, const char* s2, uint32_t n) {
    register unsigned char u1, u2;

    while (n-- > 0) {
        u1 = (unsigned char)*s1++;
        u2 = (unsigned char)*s2++;
        if (u1 != u2) return u1 - u2;
        if (u1 == '\0') return 0;
    }
    return 0;
}

char* strncpy(char* dst, const char* src, uint32_t n) {
    if (n != 0) {
        char* d = dst;
        const char* s = src;

        do {
            if ((*d++ = *s++) == 0) {
                /* NUL pad the remaining n-1 bytes */
                while (--n != 0) *d++ = 0;
                break;
            }
        } while (--n != 0);
    }
    return (dst);
}

void crash_and_burn() {
    graphics_fill(0x0000FF);
    draw_debug_console(0xFFFFFF);
    graphics_copy_backbuffer();

    disable_interrupts();
    while (1) {
        asm volatile("hlt");
    }
}

void kernel_assert(int condition, const char* file, int line, const char *error_msg) {
    if (!condition) {
        kernel_log("%s:%d assert failed: %s", file, line, error_msg);
        // hack to print stack trace
        int g = 0;
        g /= g;
        crash_and_burn();
    }
}
