// Title:	 		list.h
// Description:     handles chained lists
// First Created: 	16.12.2023
// Last Change:
 
#ifndef __LIST_H
#define __LIST_H
#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

typedef struct list_node list_node_t;

typedef struct list_node
{
    list_node_t* head;
    list_node_t* node;
    list_node_t* next;
    void* data;
};

typedef struct
{
    list_node_t* new_node;
    list_node_t* next;
    list_node_t* active;
}list_t;

int32_t init_list(list_t* list);
list_node_t* add_node(list_t* list, void* d);

#ifdef __cplusplus
}
#endif
#endif //__LIST_H