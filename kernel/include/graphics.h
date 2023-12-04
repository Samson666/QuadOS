#pragma once

// basic software graphics

#include "types.h"
#include <sgfx.h>

extern GraphicsContext graphics;

void init_graphics(u32* framebuffer, u32 width, u32 height, u32 bpp, u32 pitch);
void graphics_fill(u32 color);
void graphics_fill_rect(int32_t x, int32_t y, int32_t width, int32_t height, u32 color);
void graphics_copy_rect(int32_t xdest, int32_t ydest, int32_t width, int32_t height, int32_t xsrc, int32_t ysrc, u32* source);
void graphics_copy_backbuffer();
void graphics_draw_char(u8 c, int32_t x0, int32_t y0, u32 color);
void graphics_draw_string(const char* str, int32_t x0, int32_t y0, u32 color);
void graphics_draw_hline(int32_t x, int32_t y, int32_t w, u32 color);
void graphics_draw_vline(int32_t x, int32_t y, int32_t h, u32 color);
