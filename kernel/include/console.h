// Title:	 		console.h
// Description:
// First Created: 	19.12.2023
// Last Change:
 
#ifndef __CONSOLE_H
#define __CONSOLE_H
#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

// virtual console, outputs either to framebuffer or directly to VGA

enum VGAColor {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

typedef struct Console {
    u16 width;
    u16 height;
    uint8_t* buffer;

    u16 cursor_x;
    u16 cursor_y;

    uint8_t clear_color;
	
	int prompt_pos;
	char prompt_buffer[256];
} Console;

void init_console();
void console_print(const char* str);
void console_key_typed(char c);
void console_set_prompt_enabled(bool enabled);

#ifdef __cplusplus
}
#endif
#endif //__CONSOLE_H

