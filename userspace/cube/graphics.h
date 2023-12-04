#pragma once

#include "types.h"

typedef struct {
    u32* framebuffer;
    u32 width;
    u32 height;
    u32 bpp;
    u32 pitch;
    u32 bytesize;
} GraphicsContext;

void init_graphics(GraphicsContext* context, u32* framebuffer, u32 width, u32 height);
void graphics_fill(GraphicsContext* context, u32 color);
void graphics_line(GraphicsContext* context, int32_t x0, int32_t y0, int32_t x1, int32_t y1, u32 color);
