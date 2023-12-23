// Title:	 		list.h
// Description:
// First Created: 	21.12.2023
// Last Change:
 
#ifndef __LIST_H
#define __LIST_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct list_node list_node_t;

typedef struct list_node
{
    list_node_t* head;
    list_node_t* node;
    list_node_t* next;
    void* data;
};

#ifdef __cplusplus
}
#endif
#endif //__LIST_H

