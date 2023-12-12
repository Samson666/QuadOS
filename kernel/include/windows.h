#pragma once

// GUI window management

#include "types.h"

#define WINDOW_CONTENT_XOFFSET 1                //x-offset of the window. Used to give a distance between the windowborder and its content  
#define WINDOW_TITLE_BAR_HEIGHT 20              //height of the window title bar
#define WINDOW_STATUS_BAR_HEIGHT 10             //height of the window status bar

#define CLOSE_BUTTON_WIDTH 18                   //x dimension of the window close button
#define CLOSE_BUTTON_HEIGHT 18                  //y dimension of the window close button
#define GRIP_SIZE WINDOW_STATUS_BAR_HEIGHT      //The size of the resizing grip. For now it has the same size (width and height) as the status bar height.                           

#define WINDOW_TITLE_MAX_LENGTH 64              //The maximum length of the window title
#define WINDOW_FLAG_FULLSCREEN (1 << 0)         //Window is opened in fullscreen size
#define WINDOW_FLAG_DOUBLE_BUFFERED (1 << 1)    //Window is double buffered
#define WINDOW_FLAG_RESIZABLE (1<<2)            //Window has a resize grip
#define WINDOW_FLAG_TITLE_BAR (1<<3)            //Window has a title bar (at the top of the window)
#define WINDOW_FLAG_STATUSBAR (1<<4)            //Window has a status bar (at the bottom of the window)
#define WINDOW_FLAG_IS_WORKBENCH (1<<5)         //This window is the workbench window. Allways stay in the back

typedef struct {
    uint16_t state;                             //currently not used
    int16_t x;                                  //top left coordinates of the window
    int16_t y;                                  // ""          ""      ""
    int16_t width;                              //widht of the window width
    int16_t height;                             //content/inner height
    uint32_t flags;                             //The window flags
    int16_t actual_width;                       //currently not used
    int16_t actual_height;                      //currently not used
    uint32_t* framebuffer;                      //Pointer to the framebuffer
    uint32_t framebuffer_size_bytes;            //size of the buffer in bytes
    int16_t fb_shmem_id;                        //id of the used shared memory object
    uint8_t shown_buffer;                       //currently not used. buffer being displayed. 0 = first half, 1 = second half (starting at width * height * 4)
    char title[WINDOW_TITLE_MAX_LENGTH];        //character array for the window title
    int32_t owner_task_id;                      //pointer to the owner task
} Window;


//Global variables for windows / gui
#define MAX_WINDOWS 64                          //maximum count of windows
extern Window windows[MAX_WINDOWS];             //create maxwindow window structures (objects)

//definition of some extern/global variables
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
