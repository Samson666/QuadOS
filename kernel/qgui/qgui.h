// Title:	 		qgui.h
// Description:     header file for gui
// First Created: 	16.12.2023
// Last Change:
 
#ifndef __QGUI_H
#define __QGUI_H
#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "kmalloc.h"
#include "sharedmem.h"
#include "tasks.h"
#include "defs.h"
#include "syscall.h"
#include "interrupts.h"
#include "graphics.h"

//function declarations. Extern declaration is needed for the functions to use in C since the datatype Class is unknown in C.
//No need to load the qwindow.h file! The magic is done by the linker :-)
extern void test_qwindow_class();
extern int32_t qcreate_window(int16_t width, int16_t height, uint32_t flags);
extern void init_qgui(int32_t w, int32_t h);

class qwindow                                       //window class
{
    public: 

        //class member variables
        uint16_t state;                             //currently not used
        int16_t x;                                  //top left coordinates of the window
        int16_t y;                                  // ""          ""      ""
        int16_t width;                              //widht of the window width
        int16_t height;                             //content/inner height
        uint32_t flags;                             //the window flags
        int16_t actual_width;                       //the window width of the content
        int16_t actual_height;                      //the window height of the content
        uint32_t* framebuffer;                      //pointer to the framebuffer
        uint32_t framebuffer_size_bytes;            //size of the buffer in bytes
        int16_t fb_shmem_id;                        //id of the used shared memory object
        uint8_t shown_buffer;                       //currently not used. buffer being displayed. 0 = first half, 1 = second half (starting at width * height * 4)
        char title[WINDOW_TITLE_MAX_LENGTH];        //character array for the window title
        int32_t owner_task_id;                      //pointer to the owner task
        int32_t id;                                 //id of the window

        //class member functions            
        qwindow(){};                                //construstor
        ~qwindow(){};                               //destructor 

        void* operator new(size_t size)
        {
            return kmalloc(size);
        };

        void operator delete(void* p)
        {
            kfree(p);
        };

        //initialize the new qwindow
        void init(int16_t w, int16_t h, uint32_t f)
        {
            uint32_t fb_bytes = w * h * 4; //size of framebuffer in bytes
            state = 1;
            y = 70;
            x = 85;
            width = w;
            height = h;
            flags = f;
            actual_width = width + 2;
            actual_height = height + WINDOW_TITLE_BAR_HEIGHT + 1;
            framebuffer_size_bytes = fb_bytes;
            fb_shmem_id = sharedmem_create(fb_bytes * (flags & WINDOW_FLAG_DOUBLE_BUFFERED ? 2 : 1), 0);
            framebuffer = (uint32_t*)(fb_shmem_id, 0);
            shown_buffer = 0;
            owner_task_id = current_task->id;
            id = 1;
        };

        void draw()
        {
            // window outline
            graphics_draw_hline(x, y, width + WINDOW_CONTENT_XOFFSET * 2, COLOR_BLACK);
            graphics_draw_hline(x, y + WINDOW_TITLE_BAR_HEIGHT + height, width + WINDOW_CONTENT_XOFFSET * 2, COLOR_BLACK);
            graphics_draw_vline(x, y, height + WINDOW_TITLE_BAR_HEIGHT, COLOR_BLACK);
            graphics_draw_vline(x + width + 1, y, height + WINDOW_TITLE_BAR_HEIGHT, COLOR_BLACK);
        };
       
};

class list_node
{
    list_node* head;
    list_node* node;
    list_node* next;
    void* data;
};


class list
{
    list_node* new_node;
    list_node* next;
    list_node* active;
};


class qgui
{
    public:
    qgui(){};
    ~qgui(){};
    
    void* operator new(size_t size)
    {
        return kmalloc(size);
    };

    void operator delete(void* p)
    {
        kfree(p);
    };
    
    int32_t width, height;                  //holds the width an height of the gui
    int32_t cursor_x, cursor_y;             //holds the current mouse coordinates    
    int32_t prev_cursor_x, prev_cursor_y;   //holds the previous mouse coordinates 
    bool needs_redraw;                      //indicates if the frame needs a redraw
    bool left_click;                        //left mouse button is clicked
    bool right_click;                       //right mouse button is clicked
    u16 fake_console_buffer[50 * 80];       //buffer for the kernel console
    qwindow* active_window;
    list* window_list;

    void init(int32_t w, int32_t h)
    {
        width = w;
        height = h;
        cursor_x = w / 2;
        cursor_y = h / 2;
        prev_cursor_x = cursor_x;
        prev_cursor_y = cursor_y;
        needs_redraw = true;

        register_syscall(SYSCALL_QCREATE_WINDOW, (void*)qcreate_window);

    // init_windows();
    
    // testbutton_x = graphics.width - 100;
    // testbutton_y = graphics.height - 100;
    // testbutton_w = 50;
    // testbutton_h = 50;
    };


    void gui_task() 
    {
    // disable_interrupts();
        while (1) 
        {
            if (needs_redraw) 
            {
                needs_redraw = false;
                draw_frame();
            }

            disable_interrupts();
            handle_events();
            enable_interrupts();
        }
    };

    void draw_qwindows()
    {

    };

    void handle_events()
    {

    };

    void draw_frame()
    {
        graphics_fill(COLOR_FRAME_BACKGROUND);  //Frame with given color (00RRGGBB)
        draw_debug_console(0);
    };

    void draw_debug_console(uint32_t color) 
    {
        int index = 0;
        for (int y = 0; y < 50; y++) {
            for (int x = 0; x < 80; x++) 
            {
                u16 c = fake_console_buffer[index];
                graphics_draw_char(c & 0xFF, x * 10, y * 10, color);
                index++;
            }
        }
    };
};
    






#ifdef __cplusplus
}
#endif
#endif //__QGUI_H