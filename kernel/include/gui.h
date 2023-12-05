#pragma once

#include "types.h"

typedef struct {
    int32_t width, height;
    int32_t cursor_x, cursor_y;
    int32_t prev_cursor_x, prev_cursor_y;
    bool needs_redraw;
    bool left_click;
    u16 fake_console_buffer[50 * 80];
} GUI;

extern GUI gui;

void init_gui(int32_t width, int32_t height);
void gui_thread_entry();
void draw_debug_console(uint32_t color);
