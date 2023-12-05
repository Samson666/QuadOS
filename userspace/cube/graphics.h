#pragma once

#include "types.h"

typedef struct {
    uint32_t* framebuffer;
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
    uint32_t pitch;
    uint32_t bytesize;
} GraphicsContext;

void init_graphics(GraphicsContext* context, uint32_t* framebuffer, uint32_t width, uint32_t height);
void graphics_fill(GraphicsContext* context, uint32_t color);
void graphics_line(GraphicsContext* context, int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color);
