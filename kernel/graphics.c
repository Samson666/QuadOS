#include "graphics.h"

#include "memory.h"
#include "kmalloc.h"
#include "util.h"

GraphicsContext graphics;

static uint32_t* frontbuffer;

void init_graphics(uint32_t* framebuffer_paddr, uint32_t width, uint32_t height, uint32_t bpp, uint32_t pitch) {
    uint32_t* backbuffer = (uint32_t*) kmalloc(width * height * bpp);

    sgfx_init(&graphics, backbuffer, width, height);
    frontbuffer = (uint32_t*) KERNEL_GFX;

    uint32_t size_bytes = width * height * bpp;
    uint32_t needed_page_count = size_bytes / 0x1000 + 1;

    for (uint32_t i = 0; i < needed_page_count; i++) {
        uint32_t offset = i * 0x1000;
        mem_map_page((char*) (KERNEL_GFX + offset), ((uint32_t) framebuffer_paddr) + offset, 0);
    }
}

void graphics_fill(uint32_t color) {
    sgfx_fill(&graphics, color);
}

void graphics_fill_rect(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) {
    sgfx_fill_rect(&graphics, x, y, width, height, color);
}

void graphics_copy_rect(int32_t xdest, int32_t ydest, int32_t width, int32_t height, int32_t xsrc, int32_t ysrc, uint32_t* source) {
    sgfx_copy_rect(&graphics, xdest, ydest, width, height, xsrc, ysrc, source);
}

void graphics_copy_backbuffer() {
    int total = graphics.width * graphics.height;
    for (int i = 0; i < total; i++) {
        frontbuffer[i] = graphics.framebuffer[i];
    }
}

void graphics_draw_char(uint8_t c, int32_t x0, int32_t y0, uint32_t color) {
    sgfx_draw_char(&graphics, c, x0, y0, color);
}

void graphics_draw_string(const char* str, int32_t x0, int32_t y0, uint32_t color) {
    sgfx_draw_string(&graphics, str, x0, y0, color);
}

void graphics_draw_hline(int32_t x, int32_t y, int32_t w, uint32_t color) {
    sgfx_draw_hline(&graphics, x, y, w, color);
}

void graphics_draw_vline(int32_t x, int32_t y, int32_t h, uint32_t color) {
    sgfx_draw_vline(&graphics, x, y, h, color);
}
