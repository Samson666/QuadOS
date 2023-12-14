#include "mouse.h"

#include "util.h"
#include "interrupts.h"
#include "log.h"
#include "gui.h"

Mouse mouse;

#define PACKET_BYTES 3
static uint8_t buffer[PACKET_BYTES];
static uint8_t buffer_index;

static void wait_read();
static void wait_write();
static void write_mouse(uint8_t data);
static void handle_mouse_interrupt(TrapFrame* frame);

void init_mouse() {
    mouse.x_acc = 0;
    mouse.y_acc = 0;
    mouse.left_button = false;
    mouse.right_button = false;

    memset(buffer, 0, PACKET_BYTES);
    buffer_index = 0;
    
    register_isr(IRQ_OFFSET + 12, handle_mouse_interrupt);

    wait_write();
    outb(0x64, 0x20);

    uint8_t status = inb(0x60);
    status |= 2;
    outb(0x64, 0x60);
    outb(0x60, status);

    outb(0x64, 0xa8);

    write_mouse(0xf4);
}

static void wait_write() {
    int32_t try_count = 1000;
    while (((inb(0x64) & 2) != 0) && --try_count > 0);
}

static void write_mouse(uint8_t data) {
    wait_write();
    outb(0x64, 0xD4);

    wait_write();
    outb(0x60, data);
}

// Functionname 	: handle_mouse_interrupt
// Parameters		: none (frame is not used)
// Returns			: void
// Description		: handles the mouse interrupt
// Note				: 
 
static void handle_mouse_interrupt(TrapFrame* frame) {
    UNUSED_VAR (frame);
    
    //kernel_log("Function handle_mouse_interrupt");

    //wait for controller is ready for read registers and set it for read (0x64, 0x20)
    int32_t try_count = 1000;
    while (((inb(0x64) & 0x20) == 0) && --try_count > 0);

    //read the input buffer
    uint8_t data = inb(0x60);

    buffer[buffer_index++] = data;

    if (buffer_index >= PACKET_BYTES) {
        buffer_index = 0;

        mouse.x_acc += (int8_t) buffer[2];
        mouse.y_acc -= (int8_t) buffer[0];
        mouse.left_button = buffer[1] & 1;
        mouse.right_button = buffer[1] & 2;
        if (mouse.left_button)
            gui.left_click = true;
        if (mouse.right_button)
        {
            //kernel_log("mouse handler right click");
            gui.right_click = true;
        }

        // kernel_log("mouse x=%d, y=%d", mouse_x_acc, mouse_y_acc);
    }
}
