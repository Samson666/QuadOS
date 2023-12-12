#pragma once

// GUI window management

#include "types.h"

#define WINDOW_CONTENT_XOFFSET 1                //x-offset of the window. Used to give a distance between the windowborder and its content  
#define WINDOW_TITLE_BAR_HEIGHT 20              //height of the window title bar
#define WINDOW_STATUS_BAR_HEIGHT 10             //height of the window status bar

#define CLOSE_BUTTON_WIDTH 18                   //x dimension of the window close button
#define CLOSE_BUTTON_HEIGHT 18                  //y dimension of the window close button
#define GRIP_SIZE WINDOW_STATUS_BAR_HEIGHT      //The size of the resizing grip                           

#define WINDOW_TITLE_MAX_LENGTH 64              //The maximum length of the window title
#define WINDOW_FLAG_FULLSCREEN (1 << 0)         //Window is opened in fullscreen size
#define WINDOW_FLAG_DOUBLE_BUFFERED (1 << 1)    //Window is double buffered
#define WINDOW_FLAG_RESIZABLE (1<<2)            //Window has a resize grip
#define WINDOW_FLAG_TITLE_BAR (1<<3)            //Window has a title bar (at the top of the window)
#define WINDOW_FLAG_STATUSBAR (1<<4)            //Window has a status bar (at the bottom of the window)

typedef struct {
    uint16_t state;
    int16_t x; // top left of actual window
    int16_t y;
    int16_t width; // content/inner width
    int16_t height; // content/inner height
    uint32_t flags;
    int16_t actual_width;
    int16_t actual_height;
    uint32_t* framebuffer;
    uint32_t framebuffer_size_bytes;
    int16_t fb_shmem_id;
    uint8_t shown_buffer; // buffer being displayed. 0 = first half, 1 = second half (starting at width * height * 4)
    char title[WINDOW_TITLE_MAX_LENGTH];
    int32_t owner_task_id;
} Window;


//Global variables for windows / gui
#define MAX_WINDOWS 64                          //maximum count of windows
extern Window windows[MAX_WINDOWS];             //create maxwindow window structures (objects)

//definition of some extern variables
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
bool check_window_resize(int32_t window, int32_t x, int32_t y);
