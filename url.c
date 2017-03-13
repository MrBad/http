#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "parse_url.h"
#include "url.h"
#include "str.h"


url_t *url_new(str_t *s) 
{
	url_t *url;
	if(!(url = malloc(sizeof(*url)))) {
		perror("malloc");
		return NULL;
	}
	url->added_ts = time(NULL);
	url->url = s;
	url->parts = parse_url(s->str);
	if(!url->parts) {
		free(url);
		return NULL;
	}
	return url;
}

url_t *url_dup(url_t *url) 
{
	url_t *u;
	if(!(u = malloc(sizeof(*u)))) {
		perror("malloc");
		return NULL;
	}
	u->url = strDup(url->url);
	u->parts = url_parts_dup(url->parts);
	u->added_ts = url->added_ts;
	return u;
}

void url_del(url_t *url) 
{
	strDel(url->url);
	if(url->ip) strDel(url->ip);
	free_url_parts(url->parts);
	free(url);
}
