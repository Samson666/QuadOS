//windowtest.c
//Testing creating window in kernelmode

#include "windows.h"
#include "tasks.h"
#include "printf.h"
#include "interrupts.h"
#include "events.h"
#include "sgfx.h"
#include "sharedmem.h"

static GraphicsContext ctx;

void cwindow()
{
    disable_interrupts();               //NOTE - Important! Disable interrupts
    init_events_for_task(current_task); //NOTE - Important! Initialise events for this task
    uint32_t wid = create_window(300,300,0);           //Creating Window
    int32_t id = get_framebuffer_shmem_id(wid);
    int* fb = sharedmem_map(id, current_task->id);
    sgfx_init(&ctx, fb, 300, 300);
    enable_interrupts();                //NOTE - Important! Enable interrupts
    set_title(wid, "First Workbench window");
    sgfx_fill(&ctx, 0xFFA00000);

    while(1);                           //Endless loop to keep task alive
}
