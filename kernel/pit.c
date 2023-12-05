#include "pit.h"

#include "interrupts.h"
#include "util.h"
#include "tasks.h"
#include "events.h"

PIT pit = {0};

void handle_timer_int(TrapFrame* frame);

void init_timer(uint32_t frequency) {
    pit.freq = frequency;
    uint32_t divisor = 1193180 / frequency;

    outb(0x43, 0x36);

    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)(divisor >> 8 & 0xFF);

    outb(0x40, l);
    outb(0x40, h);

    register_isr(32, handle_timer_int);
}

void set_timer_enabled(bool enabled) {
    pit.enabled = enabled;
}

void handle_timer_int(TrapFrame* frame) {
    UNUSED_VAR(frame);
    pit.ticks++;

    if (pit.enabled) {
        check_event_timers();
        task_schedule();
    }
}