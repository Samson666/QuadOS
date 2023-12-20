// Title:	 		events.h
// Description:
// First Created: 	19.12.2023
// Last Change:
 
#ifndef __EVENTS_H
#define __EVENTS_H
#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "tasks.h"

// userspace events

enum {
    EVENT_NONE,
    EVENT_KEYBOARD,
    EVENT_MOUSE_CLICK,
    EVENT_MOUSE_MOVE,
    EVENT_TIMER,
};

typedef struct {
    uint32_t type;
    uint32_t data0;
    uint32_t data1;
    uint32_t data2;
} Event;

#define EVENT_BUFFER_SIZE 256

typedef struct {
    uint32_t num_events;
    Event buffer[EVENT_BUFFER_SIZE];
} EventBuffer;

void init_events();
void init_events_for_task(Task* task);
void destroy_events_for_task(Task* task);
void handle_event(const Event* event);
void check_event_timers();
void send_event_to_task(int32_t task_id, const Event* event);

#ifdef __cplusplus
}
#endif
#endif //__EVENTS_H


