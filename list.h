#ifndef _LIST_H
#define _LIST_H

#include "ds_common.h"

typedef struct list_node {
	void *data;
	struct list_node *prev;
	struct list_node *next;
} list_node_t;

typedef struct list {
	list_node_t *head;
	list_node_t *tail;
	unsigned int items;
	del_func_t del_func;
} list_t;


list_t *list_open(del_func_t del_func);

int list_close(list_t *list);

int list_add(list_t *list, void *data);

int list_del(list_t *list, list_node_t *node);

#define list_foreach(list, node) \
	for(node = list->head; node; node = node->next)


#endif
