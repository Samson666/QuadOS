#include "time.h"

#include "pit.h"

uint64_t get_system_time_millis() {
    // temporary
    return pit.ticks;
}
