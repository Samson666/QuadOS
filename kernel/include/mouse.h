#pragma once

#include "types.h"

// PS/2 mouse

typedef struct {
    // accumulators
    int32_t x_acc;
    int32_t y_acc;
    bool left_button;
} Mouse;

extern Mouse mouse;

void init_mouse();
