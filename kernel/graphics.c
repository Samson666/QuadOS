//Filename			: graphics.c
//First created: 12.12.2023
//Last changed		: 
//Description		: Basic graphic routines. Uses sgfx to draw to the buffer. Provides wrapper functions for sfgx
 
#include "graphics.h"

#include "memory.h"
#include "kmalloc.h"
#include "util.h"
#include "log.h"

GraphicsContext graphics;                                   //A graphics context provided by sgfx  

static uint32_t* frontbuffer;                               //Pointer to frontbuffer (buffer we will see on the screen)

// Functionname 	: init_graphics
// Parameters		: physical adress of framebuffer, width, height, bpp, pitch
// Returns			: void
// Description		: Initialize the graphics system
// Note				: 
 
void init_graphics(uint32_t* framebuffer_paddr, uint32_t width, uint32_t height, uint32_t bpp, uint32_t pitch) {
    uint32_t* backbuffer = (uint32_t*) kmalloc(width * height * bpp); //Allocate (physical)memory for our backbuffer (buffer in which we draw)

    sgfx_init(&graphics, backbuffer, width, height);        //Initialize the graphics context
    frontbuffer = (uint32_t*) KERNEL_GFX;                   //Get a pointer to the frontbuffer

    uint32_t size_bytes = width * height * bpp;             //Calculate the byte size of the framebuffer
    uint32_t needed_page_count = size_bytes / 0x1000 + 1;   //Calculate how much pages we need to map the framebuffer

    for (uint32_t i = 0; i < needed_page_count; i++) {      //mapping the framebuffer pages

        uint32_t offset = i * 0x1000;
        mem_map_page((char*) (KERNEL_GFX + offset), ((uint32_t) framebuffer_paddr) + offset, 0);
    }
}

// Functionname 	: 
// Parameters		: 
// Returns			: 
// Description		: 
// Note				: 
 
void graphics_fill(uint32_t color) {
    sgfx_fill(&graphics, color);
}

// Functionname 	: 
// Parameters		: 
// Returns			: 
// Description		: 
// Note				: 
 
void graphics_fill_rect(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color) {
    sgfx_fill_rect(&graphics, x, y, width, height, color);
}

// Functionname 	: 
// Parameters		: 
// Returns			: 
// Description		: 
// Note				: 
 
void graphics_copy_rect(int32_t xdest, int32_t ydest, int32_t width, int32_t height, int32_t xsrc, int32_t ysrc, uint32_t* source) {
    sgfx_copy_rect(&graphics, xdest, ydest, width, height, xsrc, ysrc, source);
}

// Functionname 	: 
// Parameters		: 
// Returns			: 
// Description		: 
// Note				: 
 
//Copy the backuffer (on which we draw) to framebuffer (which we see)
void graphics_copy_backbuffer() {
    uint32_t total = graphics.width * graphics.height;
    memcpy(frontbuffer, graphics.framebuffer, total<<2);
}

// Functionname 	: 
// Parameters		: 
// Returns			: 
// Description		: 
// Note				: 
 
void graphics_draw_char(uint8_t c, int32_t x0, int32_t y0, uint32_t color) {
    sgfx_draw_char(&graphics, c, x0, y0, color);
}

// Functionname 	: 
// Parameters		: 
// Returns			: 
// Description		: 
// Note				: 
 
void graphics_draw_string(const char* str, int32_t x0, int32_t y0, uint32_t color) {
    sgfx_draw_string(&graphics, str, x0, y0, color);
}

// Functionname 	: 
// Parameters		: 
// Returns			: 
// Description		: 
// Note				: 
 
void graphics_draw_hline(int32_t x, int32_t y, int32_t w, uint32_t color) {
    sgfx_draw_hline(&graphics, x, y, w, color);
}

// Functionname 	: 
// Parameters		: 
// Returns			: 
// Description		: 
// Note				: 
 
void graphics_draw_vline(int32_t x, int32_t y, int32_t h, uint32_t color) {
    sgfx_draw_vline(&graphics, x, y, h, color);
}
