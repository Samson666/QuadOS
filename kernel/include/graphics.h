// Title:	 		graphics.h
// Description:
// First Created: 	19.12.2023
// Last Change:
 
#ifndef __GRAPHICS_H
#define __GRAPHICS_H
#ifdef __cplusplus
extern "C" {
#endif

// basic software graphics

#include "types.h"
#include "sgfx.h"

extern GraphicsContext graphics;

void init_graphics(uint32_t* framebuffer, uint32_t width, uint32_t height, uint32_t bpp, uint32_t pitch);
void graphics_fill(uint32_t color);
void graphics_fill_rect(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color);
void graphics_copy_rect(int32_t xdest, int32_t ydest, int32_t width, int32_t height, int32_t xsrc, int32_t ysrc, uint32_t* source);
void graphics_copy_backbuffer();
void graphics_draw_char(uint8_t c, int32_t x0, int32_t y0, uint32_t color);
void graphics_draw_string(const char* str, int32_t x0, int32_t y0, uint32_t color);
void graphics_draw_hline(int32_t x, int32_t y, int32_t w, uint32_t color);
void graphics_draw_vline(int32_t x, int32_t y, int32_t h, uint32_t color);


#ifdef __cplusplus
}
#endif
#endif //__GRAPHICS_H

