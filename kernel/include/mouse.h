// Title:	 		mouse.h
// Description:
// First Created: 	20.12.2023
// Last Change:
 
#ifndef __MOUSE_H
#define __MOUSE_H
#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "interrupts.h"

// PS/2 mouse

typedef struct {
    // accumulators
    int32_t x_acc;
    int32_t y_acc;
    bool left_button;
    bool right_button;
} Mouse;

extern Mouse mouse;

void init_mouse();
void wait_read();
void wait_write();
void write_mouse(uint8_t data);
int8_t read_mouse();
void handle_mouse_interrupt(TrapFrame* frame);
uint8_t get_mouse_id();
void set_wheel_mode();

#ifdef __cplusplus
}
#endif
#endif //__MOUSE_H


