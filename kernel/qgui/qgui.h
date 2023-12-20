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
#include "log.h"
#include "defs.h"
#include "util.h"
#include "events.h"

//function declarations. Extern declaration is needed for C++ functions to be used in C since the datatype Class is unknown in C.
//No need to include qwindow.h file! The magic is done by the linker :-)
extern int32_t cqcreate_window(int16_t width, int16_t height, uint32_t flags);
extern void cinit_qgui(int32_t w, int32_t h);
extern void cdraw_qwindows();
extern void qhandle_event(const Event* event);
extern void qgui_task();

//Class qwindow. Handles our window
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
        bool    inside_content = false;             //mouse pointer is inside the content. Means between all other elements like titlebar etc.
        bool    mouse_over_window = false;          //mouse pointer is over the window
        uint32_t* framebuffer;                      //pointer to the framebuffer
        uint32_t framebuffer_size_bytes;            //size of the buffer in bytes
        int16_t fb_shmem_id;                        //id of the used shared memory object
        uint8_t shown_buffer;                       //currently not used. buffer being displayed. 0 = first half, 1 = second half (starting at width * height * 4)
        char title[WINDOW_TITLE_MAX_LENGTH];        //character array for the window title
        int32_t owner_task_id;                      //pointer to the owner task
        uint32_t adress;                            //pointer to this window
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
        void init(int16_t w, int16_t h, uint32_t f);
        void draw();     
};

//The class list_node. Provide our list of windows
class list_node
{
    public:

        //class member variables
        list_node* next = 0;
        void* data = 0;

        //class member functions defitions
        list_node(){};
        ~list_node(){};

        void* operator new(size_t size)
        {
            return kmalloc(size);
        };

        void operator delete(void* p)
        {
            kfree(p);
        };

        list_node* insert(void* d);   
};

//Class qgui. Handles all belongings of our gui
class qgui
{
    public:

        //member variables
        int32_t width, height;                  //holds the width an height of the gui
        int32_t cursor_x, cursor_y;             //holds the current mouse coordinates    
        int32_t prev_cursor_x, prev_cursor_y;   //holds the previous mouse coordinates
        bool    prev_mouse_left_button = false;
        bool    prev_mouse_right_button = false;
        qwindow* focused_window = NULL;
        qwindow* window_under_cursor = NULL;
        qwindow* currently_dragging_window = NULL;
        bool    mouse_inside_content = false;
        bool    needs_redraw;                      //indicates if the frame needs a redraw
        bool    left_click;                        //left mouse button is clicked
        bool    right_click;                       //right mouse button is clicked
        u16     fake_console_buffer[50 * 80];       //buffer for the kernel console
        
        list_node* list_head;
        list_node* list_tail;

        //member functions definitions
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

        
        void init(int32_t w, int32_t h);
        void append_window(qwindow* w);
        void print_window_list();
        qwindow* find_window_from_pos(int32_t x, int32_t y);
        void gui_task(); 
        void draw_qwindows();
        void handle_gui_events();
        void handle_system_events(const Event* event);
        void draw_frame();
        void draw_debug_console(uint32_t color);  
    };
    
#ifdef __cplusplus
}
#endif
#endif //__QGUI_H