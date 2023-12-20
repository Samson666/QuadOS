// Title:	 		sgfx.h
// Description:
// First Created: 	19.12.2023
// Last Change:
 
#ifndef __SGFX_H
#define __SGFX_H
#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

// basic software graphics library

typedef struct {
    uint32_t* framebuffer;
    uint32_t width;
    uint32_t height;
    uint32_t bytesize;
    uint32_t bpp;
} GraphicsContext;

void sgfx_init(GraphicsContext* ctx, uint32_t* framebuffer, uint32_t width, uint32_t height);
void sgfx_fill(const GraphicsContext* ctx, uint32_t color);
void sgfx_fill_rect(const GraphicsContext* ctx, int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color);
void sgfx_copy_rect(const GraphicsContext* ctx, int32_t xdest, int32_t ydest, int32_t width, int32_t height, int32_t xsrc, int32_t ysrc, uint32_t* source);
void sgfx_draw_char(const GraphicsContext* ctx, uint8_t c, int32_t x0, int32_t y0, uint32_t color);
void sgfx_draw_string(const GraphicsContext* ctx, const char* str, int32_t x0, int32_t y0, uint32_t color);
void sgfx_draw_hline(const GraphicsContext* ctx, int32_t x, int32_t y, int32_t w, uint32_t color);
void sgfx_draw_vline(const GraphicsContext* ctx, int32_t x, int32_t y, int32_t h, uint32_t color);
void sgfx_pixel(GraphicsContext* context, int32_t x, int32_t y, uint32_t color);
void sgfx_draw_line(GraphicsContext* context, int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color);

#ifdef __cplusplus
}
#endif
#endif //__SGFX_H



