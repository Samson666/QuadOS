#pragma once

#include "types.h"
#include "elf.h"
#include "config.h"

typedef struct {
    uint8_t state;
    char path[MAX_PATH_LENGTH];
    ELFObject elf;
    uint32_t num_users;
} SharedLibrary;

extern SharedLibrary loaded_shared_libs[MAX_SHARED_LIBRARIES];

typedef struct {
    SharedLibrary* slib;
    uint32_t offset;
} OpenSharedLibrary;

void init_shared_libs();
void init_shared_libs_for_task(uint32_t task_id, ELFObject* elf);
