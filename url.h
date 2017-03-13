#ifndef _URL_H
#define _URL_H
#include "str.h"
#include "parse_url.h"

typedef struct {
	str_t *url;
	url_parts_t *parts;
	char *ip;
	time_t added_ts;
} url_t;


url_t *url_new(str_t *str);
void  url_del(url_t *url);
url_t *url_dup(url_t *url);
void url_toString(url_t *url);
#endif
