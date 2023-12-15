#pragma once

#include "types.h"
#include "windows.h"

typedef struct
{
    Window* head;
    Window* node;
    Window* next;
}list_node_t;


typedef struct
{
    list_node_t* new_node;                         //head of the list
    list_node_t* next;                         //last window in list
    list_node_t* active;                       //the active window (former focused window)
}list_t;

typedef struct {
    int32_t width, height;                  //holds the width an height of the gui
    int32_t cursor_x, cursor_y;             //holds the current mouse coordinates    
    int32_t prev_cursor_x, prev_cursor_y;   //holds the previous mouse coordinates 
    bool needs_redraw;                      //indicates if the frame needs a redraw
    bool left_click;                        //left mouse button is clicked
    bool right_click;                       //right mouse button is clicked
    u16 fake_console_buffer[50 * 80];       //buffer for the kernel console
    Window* active_window;
    list_t* window_list;
} GUI;

extern GUI gui;

void init_gui(int32_t width, int32_t height);
void gui_task();
void draw_debug_console(uint32_t color);
