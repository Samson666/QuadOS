#include <os.h>
#include <types.h>
#include <string.h>
#include <sgfx.h>
#include <stdio.h>
#include "windows.h"
#include "defs.h"

#define width 400
#define height 320

#define MAX_ENTRIES 64

static void handle_mouse_move(int x, int y);
static void handle_left_click();

static GraphicsContext ctx;

int main(int argc, char* argv[]) 
{
    int window = os_create_window(width, height, (unsigned int) WINDOW_FLAG_RESIZABLE);
    os_set_window_title(window, "Task Manager");
    int* fb = os_map_window_framebuffer(window);

    sgfx_init(&ctx, fb, width, height);

    os_resize_window(window, 200, 200);
    sgfx_init(&ctx, fb, 200,200 );

    unsigned int prev_left_mouse_state = 0;
    uint32_t last_mouse_button_state = 0;

        
        
    OSEvent event;
    while (1) {
        sgfx_fill(&ctx, COLOR_WINDOW_BACKGROUND);
        sgfx_draw_string(&ctx, "Tasks:", 3, 5, 0xFFFFFFFF);


        os_swap_window_buffers(window);
        os_wait_for_events();
        
        while (os_poll_event(&event)) 
        {
            if (event.type == OS_EVENT_MOUSE_MOVE || event.type == OS_EVENT_MOUSE_CLICK) {
                OSMouseEvent* e = (OSMouseEvent*) &event;

                //handle_mouse_move(e->x, e->y);

                if (e->buttons) {
                    handle_left_click(e->x, e->y);
                }

                last_mouse_button_state = e->buttons;
            }
        }
    }
    return 0;
}

static void handle_mouse_move(int x, int y) 
{
 
}

static void handle_left_click(int x, int y) 
{
    os_print("Mouse clicked");
}
