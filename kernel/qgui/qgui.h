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
            id = 0;
        };

        void operator delete(void* p)
        {
            kfree(p);
        };
    
       
};

//funktion declarations
extern void test_qwindow_class();

#ifdef __cplusplus
}
#endif
#endif //__QGUI_H