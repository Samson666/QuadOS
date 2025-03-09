#include "time.h"
#include "pit.h"
#include "log.h"
#include "util.h"
#include "tasks.h"

volatile int64_t sleep_counter = 0;

uint64_t get_system_time_millis() {
    // temporary
    return pit.ticks;
}

void sleep_task()
{
    while(1){sleep_counter = get_system_time_millis();};
}

void sleep(uint64_t sleep_millis)
{
    sleep_counter = sleep_millis;
    int32_t id=create_kernel_task(sleep_task);
    while(sleep_counter <= sleep_millis+sleep_counter);
    kill_task(id);    
}


