//Filename			: list.c
//First created: 16.12.2023
//Last changed		: 
//Description		: handles chained lists

#include "list.h"
#include "types.h"

int32_t init_list(list_t* list)
{
    list->new_node = 0;
    list->next = 0;
    list->active = 0;
}

list_node_t* add_node(list_t* list, void* d)
{
    list->new_node = kmalloc(sizeof(list_node_t));
    list->new_node->next = list->next;
    list->next = list->new_node;
    list->new_node->data = d;
    return(list->new_node);
}
 