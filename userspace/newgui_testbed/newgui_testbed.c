//Filename			: newgui_testbed.c
//First created     : 17.12.2023
//Last changed		: 
//Description		: Testing the new gui


 
#include <os.h>
#include <types.h>
#include <string.h>
#include <sgfx.h>
#include <stdio.h>
#include "windows.h"
#include "defs.h"
#include "stdio.h"

#define width 400
#define height 320

#define MAX_ENTRIES 64
#define FILE_LIST_Y_OFFSET 32
#define FILE_LIST_Y_SPACING 12

static GraphicsContext ctx;

int main(int argc, char* argv[]) {
    int window = os_qcreate_window(width, height, (unsigned int) WINDOW_FLAG_RESIZABLE);
    //os_set_window_title(window, "Newgui Testbed");
    int* fb = os_map_window_framebuffer(window);

    os_printf("newgui_testbed.exe window id: %d\n", window);
    sgfx_init(&ctx, fb, width, height);

   
    while (1) 
    {
    
    }
        
    return 0;
}
