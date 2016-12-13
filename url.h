#ifndef _URL_H_
#define _URL_H_


#include <stdlib.h>
#include <time.h>
#include "list.h"
#include "parse_url.h"

typedef struct url_t {
	char *url;
	url_parts_t *parts;	
	time_t added_ts;
} url_t;


url_t *new_url(char *str_url); 

void delete_url(void *url);

// function used in lists / callbacks
int print_url(void *_url);

//int find_url(void *_url, void *_compared_url);


url_t *url_clone(url_t *inurl);


#endif
