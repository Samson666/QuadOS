// Title:	 		interrupts.h
// Description:
// First Created: 	16.12.2023
// Last Change:
 
#ifndef __INTERRUPTS_H
#define __INTERRUPTS_H
#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

#define IRQ_OFFSET 32

#define enable_interrupts() asm volatile("sti")
#define disable_interrupts() asm volatile("cli")
#define FL_IF 0x200 // interrupt enable

#define VERIFY_INTERRUPTS_DISABLED assert(!are_interrupts_enabled())

typedef struct {
    u16 isr_low;
    u16 kernel_cs;
    uint8_t  reserved;
    uint8_t  attributes;
    u16 isr_high;
} __attribute__((packed)) IDTEntry;

typedef struct {
    u16 limit;
    uint32_t base;
} __attribute__((packed)) IDTPointer;

typedef struct {
    // pushed by us:
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // esp is ignored
    uint32_t interrupt, error;

    // pushed by the CPU:
    uint32_t eip, cs, eflags, usermode_esp, usermode_ss;
} TrapFrame;

typedef struct {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebx;
    uint32_t ebp;
    uint32_t eip;
} TaskSwitchContext;

typedef void (*ISRFunction)(TrapFrame*);

void setup_interrupts();
void register_isr(uint8_t vector, ISRFunction func);
void handle_interrupt(TrapFrame* frame);
void push_cli();
void pop_cli();
bool are_interrupts_enabled();

#ifdef __cplusplus
}
#endif
#endif //__INTERRUPTS_H
