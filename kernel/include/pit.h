#pragma once

#include "types.h"

typedef struct {
    uint64_t ticks;
    uint32_t freq;
    bool enabled;
} PIT;

extern PIT pit;

void init_timer(uint32_t frequency);
void set_timer_enabled(bool enabled);
