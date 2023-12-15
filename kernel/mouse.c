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
static uint8_t get_mouse_id();
static void set_wheel_mode();

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

    //set_wheel_mode();

    write_mouse(0xf4);
    
}

static void wait_write() {
    int32_t try_count = 1000;
    while (((inb(0x64) & 2) != 0) && --try_count > 0);
}

static void wait_read()
{
    int32_t try_count = 1000;
    while (((inb(0x64) & 0x20) == 0) && --try_count > 0);
}

static void write_mouse(uint8_t data) {
    wait_write();
    outb(0x64, 0xD4);

    wait_write();
    outb(0x60, data);
}

static int8_t read_mouse()
{
    wait_read();
    return(inb(0x60));
}

void set_sample_rate(uint8_t sample_rate)
{
    uint8_t ack;
    outb(0xD4, 0x64);                    // tell the controller to address the mouse
    outb(0xF3, 0x60);                    // write the mouse command code to the controller's data port
    
    wait_read(); // wait until we can read
    
    while(inb(0x60)!=0xfa);                     // read back acknowledge. This should be 0xFA
    outb(0xD4, 0x64);                    // tell the controller to address the mouse
    outb(sample_rate, 0x60);                     // write the parameter to the controller's data port
    wait_read(); // wait until we can read
    while(inb(0x60)!=0xfa); // read back acknowledge. This should be 0xFA
}

//not working!
void set_wheel_mode()
{
    set_sample_rate(200);
    set_sample_rate(100);
    set_sample_rate(80);
    write_mouse(0xf2);
    uint8_t x = read_mouse();

}

// Functionname 	: handle_mouse_interrupt
// Parameters		: none (frame is not used)
// Returns			: void
// Description		: handles the mouse interrupt
// Note				: 
 
static void handle_mouse_interrupt(TrapFrame* frame) {
    UNUSED_VAR (frame);

    uint8_t scroll = 0;
    uint8_t data = read_mouse();

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
            gui.right_click = true;

        // kernel_log("mouse x=%d, y=%d", mouse_x_acc, mouse_y_acc);
    }
}
