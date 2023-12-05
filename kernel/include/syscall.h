#pragma once

#include "syscall_list.h"
#include "types.h"

void init_syscalls();
void register_syscall(uint32_t vector, void* func);
