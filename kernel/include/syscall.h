// Title:	 		syscall.h
// Description:
// First Created: 	16.12.2023
// Last Change:
 
#ifndef __SYSCALL_H
#define __SYSCALL_H
#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

void init_syscalls();
void register_syscall(uint32_t vector, void* func);

#ifdef __cplusplus
}
#endif
#endif //__SYSCALL_H

#include "syscall_list.h"
#include "types.h"


