#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

queue_t *queue_open() {
	queue_t *queue;
	queue = (queue_t *) malloc(sizeof(queue_t));

	if(!queue) {
		perror("malloc");
		return NULL;
	}
	queue->num_items = 0;
	return queue;
}

int queue_close(queue_t *queue) {
	queue_item_t *node;
	node = queue->tail;
	while(node) {
		free(node);
		node = node->prev;
	}
	free(queue);
	return 0;
}

queue_item_t *new_node(void *data) {
	queue_item_t *n; 
	n = (queue_item_t *) malloc(sizeof(queue_item_t));
	if(!n) {
		perror("malloc");
		return NULL;
	}   
	n->data = (void *) data;
	n->prev = n->next = NULL;
	return n;
}

void queue_add(queue_t *queue, queue_item_t *node) {
	if(!queue->head) {
		queue->head = node;
		queue->tail = node;
	} else {
		queue->tail->next = node;
		node->prev = queue->tail;
		queue->tail = node;
	}
	queue->num_items++;
}

void queue_del(queue_t *queue, queue_item_t *node) {
	queue_item_t *prev, *next;
	prev = node->prev;
	next = node->next;
	if(prev) {
		prev->next = next;
	} else {
		queue->head = next;
	}
	if(next) {
		next->prev = prev;
	} else {
		queue->tail = prev;
	}
	free(node);
	queue->num_items--;
}


