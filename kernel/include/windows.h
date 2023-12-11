#pragma once

// GUI window management

#include "types.h"

#define WINDOW_CONTENT_XOFFSET 1
#define WINDOW_CONTENT_YOFFSET 20

#define CLOSE_BUTTON_WIDTH 18
#define CLOSE_BUTTON_HEIGHT 18

#define WINDOW_TITLE_MAX_LENGTH 64
#define WINDOW_FLAG_FULLSCREEN (1 << 0)
#define WINDOW_FLAG_DOUBLE_BUFFERED (1 << 1)

typedef struct {
    u16 state;
    s16 x; // top left of actual window
    s16 y;
    s16 width; // content/inner width
    s16 height; // content/inner height
    uint32_t flags;
    s16 actual_width;
    s16 actual_height;
    uint32_t* framebuffer;
    uint32_t framebuffer_size_bytes;
    s16 fb_shmem_id;
    uint8_t shown_buffer; // buffer being displayed. 0 = first half, 1 = second half (starting at width * height * 4)
    char title[WINDOW_TITLE_MAX_LENGTH];
    int32_t owner_task_id;
} Window;


//Global variables for windows / gui
#define MAX_WINDOWS 64
extern Window windows[MAX_WINDOWS];

extern int32_t currently_dragging_window;
extern int32_t focused_window;
extern int32_t window_under_cursor;
extern bool window_under_cursor_inside_content;

//function declarations
void init_windows();
int32_t create_window(int32_t width, int32_t height, uint32_t flags);
void destroy_window(int32_t window_id);
bool check_window_close(int32_t window, int32_t x, int32_t y);
int32_t find_window_from_pos(int32_t x, int32_t y, bool* inside_content);
Window* get_window(int32_t id);
void move_window_to_front(int32_t window);
int32_t get_framebuffer_shmem_id(int32_t window_id);
int32_t set_title(int32_t window_id, const char* title);
void resize_window(int32_t id, int32_t width, int32_t height);
