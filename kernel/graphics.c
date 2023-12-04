#include "graphics.h"

#include "memory.h"
#include "kmalloc.h"
#include "util.h"

GraphicsContext graphics;

static u32* frontbuffer;

void init_graphics(u32* framebuffer_paddr, u32 width, u32 height, u32 bpp, u32 pitch) {
    u32* backbuffer = (u32*) kmalloc(width * height * bpp);

    sgfx_init(&graphics, backbuffer, width, height);
    frontbuffer = (u32*) KERNEL_GFX;

    u32 size_bytes = width * height * bpp;
    u32 needed_page_count = size_bytes / 0x1000 + 1;

    for (u32 i = 0; i < needed_page_count; i++) {
        u32 offset = i * 0x1000;
        mem_map_page((char*) (KERNEL_GFX + offset), ((u32) framebuffer_paddr) + offset, 0);
    }
}

void graphics_fill(u32 color) {
    sgfx_fill(&graphics, color);
}

void graphics_fill_rect(int32_t x, int32_t y, int32_t width, int32_t height, u32 color) {
    sgfx_fill_rect(&graphics, x, y, width, height, color);
}

void graphics_copy_rect(int32_t xdest, int32_t ydest, int32_t width, int32_t height, int32_t xsrc, int32_t ysrc, u32* source) {
    sgfx_copy_rect(&graphics, xdest, ydest, width, height, xsrc, ysrc, source);
}

void graphics_copy_backbuffer() {
    int total = graphics.width * graphics.height;
    for (int i = 0; i < total; i++) {
        frontbuffer[i] = graphics.framebuffer[i];
    }
}

void graphics_draw_char(u8 c, int32_t x0, int32_t y0, u32 color) {
    sgfx_draw_char(&graphics, c, x0, y0, color);
}

void graphics_draw_string(const char* str, int32_t x0, int32_t y0, u32 color) {
    sgfx_draw_string(&graphics, str, x0, y0, color);
}

void graphics_draw_hline(int32_t x, int32_t y, int32_t w, u32 color) {
    sgfx_draw_hline(&graphics, x, y, w, color);
}

void graphics_draw_vline(int32_t x, int32_t y, int32_t h, u32 color) {
    sgfx_draw_vline(&graphics, x, y, h, color);
}
