#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include "ds_common.h"

list_t *
list_open(del_func_t del_func) 
{
	list_t *list;
	if(!(list = malloc(sizeof(*list)))) {
		perror("malloc");
		return NULL;
	}
	list->head = list->tail = NULL;
	list->items = 0;
	list->del_func = del_func;
	return list;
}

int list_close(list_t *list) 
{
	list_node_t *n;
	while(list->head) {
		n = list->head->next;
		if(list->del_func)
			list->del_func(list->head->data);
		free(list->head);
		list->head = n;
	}
	return 0;
}

list_node_t *list_node_new(void *data) 
{
	list_node_t *n;
	if(!(n = malloc(sizeof(*n)))) {
		perror("malloc");
		return NULL;
	}
	n->data = data;
	n->prev = n->next = NULL;
	return n;
}

int list_add(list_t *list, void *data) 
{
	list_node_t *n;
	if(!data)
		return -1;
	if(!(n = list_node_new(data))) {
		return -1;
	}
	if(!list->head)
		list->head = list->tail = n;
	else {
		n->prev = list->tail;
		list->tail->next = n;
		list->tail = n;
	}
	list->items++;
	return 0;
}

int list_del(list_t *list, list_node_t *node) 
{
	list_node_t *prev, *next;
	prev = node->prev;
	next = node->next;
	if(prev)
		prev->next = next;
	else
		list->head = next;
	if(next)
		next->prev = prev;
	else
		list->tail = prev;	
	if(list->del_func)
		list->del_func(node->data);
	free(node);
	list->items--;
	return 0;
}
