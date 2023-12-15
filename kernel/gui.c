//Filename			: gui.c
//First created: 12.12.2023
//Last changed		: 
//Description		: 
 
#include "gui.h"

#include "graphics.h"
#include "util.h"
#include "log.h"
#include "sharedmem.h"
#include "syscall.h"
#include "mouse.h"
#include "tasks.h"
#include "events.h"
#include "windows.h"
#include "res/cursor_img.h"
#include "time.h"
#include "interrupts.h"
#include "defs.h"

GUI gui;

int32_t testbutton_x, testbutton_y;
int32_t testbutton_w, testbutton_h;

int32_t redraw_indicator = 0;

static void gui_handle_events();
static void gui_draw_frame();

void init_gui(int32_t width, int32_t height) {
    gui.width = width;
    gui.height = height;
    gui.cursor_x = width / 2;
    gui.cursor_y = height / 2;
    gui.prev_cursor_x = gui.cursor_x;
    gui.prev_cursor_y = gui.cursor_y;
    gui.needs_redraw = true;

    init_windows();
    
    testbutton_x = graphics.width - 100;
    testbutton_y = graphics.height - 100;
    testbutton_w = 50;
    testbutton_h = 50;
}

//The GUI task
void gui_task() {
    // disable_interrupts();
    while (1) {
        if (gui.needs_redraw) {
            gui.needs_redraw = false;
            gui_draw_frame();
        }

        disable_interrupts();
        gui_handle_events();
        enable_interrupts();
    }
}

//Draw the complete frame (screen)
static void gui_draw_frame() {
    graphics_fill(COLOR_FRAME_BACKGROUND);  //Frame with given color (00RRGGBB)

    draw_debug_console(0);
    draw_windows();             //drawing the windows

    graphics_fill_rect(testbutton_x, testbutton_y, testbutton_w, testbutton_h, 0xFFFF00FF); //Drawing the testbutton to start file.exe

    graphics_fill_rect(graphics.width - 10, 2, 8, 8, redraw_indicator ? 0xFF00FF : 0); //Drawing the indicator rectangle
    redraw_indicator ^= 1; //Invers the indicator color

    //get time and save it time_str
    uint64_t time = get_system_time_millis(); //Get the system time in milliseconds
    char time_str[128];

    int phys_mem = pmm_get_total_allocated_pages() * 4; //get used physical memory

    sprintf(time_str, "phys used: %dKiB   systime: %u", phys_mem, time);
    graphics_draw_string(time_str, 3, graphics.height - 15, 0); //Print the system time at the bottom of the frame

    graphics_copy_rect(gui.cursor_x, gui.cursor_y, 12, 19, 0, 0, res_cursor_raw); //draw the cursor

    //Since we draw everything to the backbuffer, we need to copy the backbuffer to the actual framebuffer
    graphics_copy_backbuffer();
}

static void handle_left_click() {
    //Testbutton was clicked -> start files.exe in a new usertask
    if (gui.cursor_x >= testbutton_x && gui.cursor_x < testbutton_x + testbutton_w
        && gui.cursor_y >= testbutton_y && gui.cursor_y < testbutton_y + testbutton_h) {
        create_user_task("files.exe");
    }

    focused_window = window_under_cursor;

    if (window_under_cursor != -1) {
        // we are clicking on a window
        move_window_to_front(window_under_cursor);      //bring the window to front

        if (window_under_cursor_inside_content) {
            Event click;
            click.type = EVENT_MOUSE_CLICK;
            click.data0 = gui.cursor_x - windows[window_under_cursor].x - WINDOW_CONTENT_XOFFSET;
            click.data1 = gui.cursor_y - windows[window_under_cursor].y - WINDOW_TITLE_BAR_HEIGHT;
            click.data2 = 1;
            handle_event(&click);
        } else {
            // we are clicking on its border
            
            if(check_window_resize(window_under_cursor, gui.cursor_x, gui.cursor_y))
            {
                kernel_log("Window resize grip clicked");
            }
            
            else if (check_window_close(window_under_cursor, gui.cursor_x, gui.cursor_y))
            {
                Window* w = get_window(window_under_cursor);
                uint32_t task_id = w->owner_task_id;
                // todo: allow for multiple windows
                destroy_window(window_under_cursor);
                kill_task(task_id);
                window_under_cursor = -1;
                focused_window = -1;
            }

            currently_dragging_window = window_under_cursor;
            
        }
        
    }

    gui.needs_redraw = true;
}
// Functionname 	: handle_right_click
// Parameters		: none
// Returns			: void
// Description		: handles the mouse right click
// Note				: 
 
static void handle_right_click()
{

    kernel_log("Mouse right click");
}


// Functionname 	: gui_handle_events
// Parameters		: none
// Returns			: void
// Description		: handles the events in gui thread loop (gui_task in this file)
// Note				: 
 
static void gui_handle_events() {
    //get the movement of the mousepointer
    gui.cursor_x += mouse.x_acc;
    mouse.x_acc = 0;
    gui.cursor_y += mouse.y_acc;
    mouse.y_acc = 0;

    //check if mousepointer is in the boundaries.
    if (gui.cursor_x < 0)
        gui.cursor_x = 0;
    if (gui.cursor_y < 0)
        gui.cursor_y = 0;
    if (gui.cursor_x >= graphics.width)
        gui.cursor_x = graphics.width - 1;
    if (gui.cursor_y >= graphics.height)
        gui.cursor_y = graphics.height - 1;

    //get the window which is currently under the cursor
    if(gui.cursor_x != gui.prev_cursor_x && gui.cursor_y != gui.prev_cursor_y)
        window_under_cursor = find_window_from_pos(gui.cursor_x, gui.cursor_y, &window_under_cursor_inside_content);

    static bool prev_mouse_left_button = 0;
    static bool prev_mouse_right_button = 0;

    if (prev_mouse_left_button != gui.left_click || prev_mouse_right_button != gui.right_click)
    {
        if (gui.left_click) {
            handle_left_click();
        }
        else if (gui.right_click){
            handle_right_click();
        } 
        else {
            currently_dragging_window = -1;
        }

        prev_mouse_left_button = gui.left_click;
        prev_mouse_right_button = gui.right_click;
    }
    
    gui.left_click = mouse.left_button;
    gui.right_click = mouse.right_button;

    int32_t dx = gui.cursor_x - gui.prev_cursor_x;
    int32_t dy = gui.cursor_y - gui.prev_cursor_y;

    if (dx != 0 || dy != 0) {
        gui.needs_redraw = true;

        if (currently_dragging_window != -1) {
            windows[currently_dragging_window].x += dx;
            windows[currently_dragging_window].y += dy;
        } else if (window_under_cursor_inside_content) { // && focused_window == window_under_cursor
            Event move;
            move.type = EVENT_MOUSE_MOVE;
            move.data0 = gui.cursor_x - windows[window_under_cursor].x - WINDOW_CONTENT_XOFFSET;
            move.data1 = gui.cursor_y - windows[window_under_cursor].y - WINDOW_TITLE_BAR_HEIGHT;
            move.data2 = 0;
            handle_event(&move);
        }

        gui.prev_cursor_x = gui.cursor_x;
        gui.prev_cursor_y = gui.cursor_y;
    }
}

void draw_debug_console(uint32_t color) {
    int index = 0;
    for (int y = 0; y < 50; y++) {
        for (int x = 0; x < 80; x++) {
            u16 c = gui.fake_console_buffer[index];
            graphics_draw_char(c & 0xFF, x * 10, y * 10, color);
            index++;
        }
    }
}
