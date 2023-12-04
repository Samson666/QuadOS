#pragma once

#include "types.h"

// basic software graphics library

typedef struct {
    u32* framebuffer;
    u32 width;
    u32 height;
    u32 bytesize;
} GraphicsContext;

void sgfx_init(GraphicsContext* ctx, u32* framebuffer, u32 width, u32 height);
void sgfx_fill(const GraphicsContext* ctx, u32 color);
void sgfx_fill_rect(const GraphicsContext* ctx, int32_t x, int32_t y, int32_t width, int32_t height, u32 color);
void sgfx_copy_rect(const GraphicsContext* ctx, int32_t xdest, int32_t ydest, int32_t width, int32_t height, int32_t xsrc, int32_t ysrc, u32* source);
void sgfx_draw_char(const GraphicsContext* ctx, u8 c, int32_t x0, int32_t y0, u32 color);
void sgfx_draw_string(const GraphicsContext* ctx, const char* str, int32_t x0, int32_t y0, u32 color);
void sgfx_draw_hline(const GraphicsContext* ctx, int32_t x, int32_t y, int32_t w, u32 color);
void sgfx_draw_vline(const GraphicsContext* ctx, int32_t x, int32_t y, int32_t h, u32 color);
void sgfx_pixel(GraphicsContext* context, int32_t x, int32_t y, u32 color);
void sgfx_draw_line(GraphicsContext* context, int32_t x0, int32_t y0, int32_t x1, int32_t y1, u32 color);

