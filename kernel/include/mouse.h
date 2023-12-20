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

#ifdef __cplusplus
}
#endif
#endif //__MOUSE_H


