#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "list.h"

list_t * list_open(free_function_t free_function)
{
	list_t *list;
	list = (list_t *) malloc(sizeof(list_t));
	if(!list) {
		perror("malloc");
		return NULL;
	}
	list->head = list->tail = NULL;
	list->num_items = 0;
	list->free_function = free_function;

	return list;
}

node_t *new_node(void *data) 
{
	node_t *node;
	node = (node_t *) malloc(sizeof(node_t));
	node->next = node->prev = NULL;
	node->data = data;
	return node;
}

void list_add_node(list_t *list, node_t *node) 
{
	if(!list->head) {
		list->head = list->tail = node;	
	} else {
		node->prev = list->tail;
		list->tail->next = node;
		list->tail = node;
	}
	list->num_items++;
}

node_t *list_add(list_t *list, void *data) {
	node_t *node;
	node = new_node(data);
	list_add_node(list, node);
	return node;
}

void list_del(list_t *list, node_t *node) {
	node_t *prev, *next;
	prev = node->prev;
	next = node->next;
	if(prev) {
		prev->next = next;
	} else {
		list->head = next;
	}
	if(next) {
		next->prev = prev;
	} else {
		list->tail = prev;
	}
	if(list->free_function) {
		list->free_function(node->data);
	}
	free(node);
	list->num_items--;
}

void list_close(list_t *list) 
{
	node_t *node;

	while(list->head) {
		node = list->head;
		if(list->free_function) {
			list->free_function(node->data);
		}
		free(node);
		list->head = node->next;
	}
	free(list);
}

#if 0
node_t *list_for_each(list_t *list, iterator_function_t iterator, ...) 
{
	node_t *last_node = NULL;
   	node_t *node;
	int found;
	node = list->head;

	va_list ap;

	while(node) {
		va_start(ap, iterator);
		found = iterator(node, &ap);
		va_end(ap);
		if(found) {
			last_node = node;
			break;
		}
		node = node->next;
	}
	return last_node;	
}
#endif 

node_t *list_for_each(list_t *list, iterator_function_t iterator) 
{
	node_t *last_node = NULL;
   	node_t *node;
	int found;
	node = list->head;

	while(node) {
		found = iterator(node);
		if(found) {
			last_node = node;
			break;
		}
		node = node->next;
	}
	return last_node;	
}

node_t *list_find(list_t *list, compare_function_t compare, void *what) {
	node_t *last_node = NULL;
	node_t *node;
	
	int found;
	node = list->head;
	while(node) {
		found = compare(node, what);
		if(found) {
			last_node = node;
		}
		node = node->next;
	}
	return last_node;
}


