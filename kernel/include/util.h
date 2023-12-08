#pragma once

#include "types.h"

void* memset(void* vdest, uint8_t val, uint32_t len);
void* memcpy(void* dest, const void* src, size_t bytes);
uint32_t strcmp(const char* s1, const char* s2);
uint32_t strncmp(const char* s1, const char* s2, uint32_t n);
char* strncpy(char* dst, const char* src, uint32_t n);
size_t strlen(const char *str);
char* strcpy(char* dest, const char* src);

static inline void outb(u16 port, uint8_t value) {
    asm volatile("outb %1, %0" :: "dN" (port), "a" (value));
}

static inline uint8_t inb(u16 port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}

#define UNUSED_VAR(x) (void) x
#define halt() asm volatile("hlt")

void crash_and_burn();
void kernel_assert(int condition, const char* file, int line, const char* error_msg);

// todo: pass vargs
#define assert_msg(x, msg) kernel_assert(x, __FILE__, __LINE__, msg)
#define assert(x) kernel_assert(x, __FILE__, __LINE__, "")

#define CEIL_DIV(a, b) (((a) + (b) - 1) / (b))
#define CLAMP(x, a, b) (((x) < (a)) ? (a) : ((x) > (b) ? (b) : (x)))
#define MIN(x, y) (((x) < (y)) ?  (x) : (y))
#define MAX(x, y) (((x) > (y)) ?  (x) : (y))

static inline uint32_t read_eflags() {
    uint32_t eflags;
    asm volatile(
        "pushfl \n"
        "popl %0 \n"
        : "=r" (eflags)
    );
    return eflags;
}
