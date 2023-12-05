#include "types.h"
#include "log.h"
#include "gdt.h"
#include "interrupts.h"
#include "util.h"
#include "pit.h"
#include "keyboard.h"
#include "console.h"
#include "multiboot.h"
#include "memory.h"
#include "kmalloc.h"
#include "physalloc.h"
#include "tasks.h"
#include "exceptions.h"
#include "syscall.h"
#include "elf.h"
#include "ramdisk.h"
#include "sharedmem.h"
#include "events.h"
#include "graphics.h"
#include "gui.h"
#include "mouse.h"
#include "fpu.h"
#include "slib.h"

bool graphics_enabled;

void kernel_main(struct multiboot_info* info) {
    disable_interrupts();               //disable all interrupts that we are not disturbed while changing gdt/idt
    setup_gdt();                        //Setting up the global descriptor table

    graphics_enabled = info->framebuffer_type == MULTIBOOT_FRAMEBUFFER_TYPE_RGB; //Set graphics to true if GRUB gave us a valid framebuffer

    
    init_logging(true);                     //Initialize the logging to stdout
    kernel_log("Wellcome to QuadOS. Warming up");               //Print out a wellcome Message to stdout

    u32 mod_count = info->mods_count;       //Get the number of modules from the multiboot structure
    assert_msg(mod_count, "zero modules!"); //No modules found? -> exit
    u32 mod0 = *(u32*) (info->mods_addr);   //Getting the adress of the GRUB modules
    u32 mod1 = *(u32*) (info->mods_addr + 4);

    u32 framebuffer_addr = info->framebuffer_addr;      //Getting the framebuffer address from the multiboot structure
    u32 framebuffer_width = info->framebuffer_width;    //Getting the framebuffer width from the multiboot structure
    u32 framebuffer_height = info->framebuffer_height;  //Getting the framebuffer height from the multiboot structure
    u32 framebuffer_bpp = info->framebuffer_bpp;        //Getting the framebuffer bits per pixel from the multiboot structure
    u32 framebuffer_pitch = info->framebuffer_pitch;    //Getting the framebuffer pixel pitch from the multiboot structure

    kernel_log("Available memory: %uMB", info->mem_upper / 1024);

    kernel_log("Setting up interrupts");                
    setup_interrupts();                                 //Set up the Interrupts
    init_exceptions();                                  //Setup the exceptions                        
    init_syscalls();                                    //Initalize the syscalls

    //enable_fpu();

    init_timer(1000);                                   //initialise the timer
    
    init_memory(info->mem_upper * 1024);                //Initialise the memory            
    kernel_log("Initial pagedir is at %x", mem_get_current_page_directory());
    kernel_log("Setting up kernel heap");
    kmalloc_init(0x1000);                               //Initialise the heap

    setup_tasks();                                      //Setting up the task system        
    sharedmem_init();                                   //Initialise the shared memory 
    init_shared_libs();                                 //Initialise the shared librarys
    init_events();                                      //Init events

    u32 ramdisk_size = mod1 - mod0;                     //Getting the Ramdisk size (from GRUB Module)
    init_ramdisk(mod0 + 0xC0000000, ramdisk_size);      //Initialise the Ramdisk

    //If graphics is enabled:
    if (graphics_enabled) {
        init_graphics((u32*) framebuffer_addr, framebuffer_width, framebuffer_height, (u32) framebuffer_bpp / 8, framebuffer_pitch);
                                                        //Initialise the graphics
        init_gui(framebuffer_width, framebuffer_height);//Initialise the graphical user interface
        create_kernel_task(gui_thread_entry);           //Create the kernel task for the GUI

        create_user_task("files.exe");                  //Starting task from file
    }

    init_keyboard();                                    //Initalise the keyboard
    init_mouse();                                       //Initalise the mouse

    set_timer_enabled(true);                            //Enable the timer    
    
           

    console_set_prompt_enabled(true);                   //Enable the prompt in the graphical console
    enable_interrupts();                                //Enable interrupts
    
    kernel_log("QuadOS is up and running");             //Message success to stdout 

    //Idle loop for kernel task (this task)
    while (true) {
        halt();
    }
}
