#include <stdio.h>
#include <string.h>
#include <time.h>
#include "url.h"
#include "parse_url.h"

url_t *new_url(char *str_url)
{
	url_t *url;
	url = (url_t *) malloc(sizeof(url_t));
	if(!url) {
		perror("malloc");
		return NULL;
	}

	url->url = strdup(str_url);
	url->added_ts = time(NULL);
	url->parts = parse_url(str_url);
	if(!url->parts) {
		return NULL;
	}
	
	return url;
}

url_t *url_clone(url_t *inurl) {
	url_t *url;
	url = (url_t *) malloc(sizeof(url_t));
	if(!url) {
		perror("malloc");
		return NULL;
	}
	url->url = strdup(inurl->url);
	url->parts = (url_parts_t *)malloc(sizeof(url_parts_t));
	memcpy(url->parts, inurl->parts, sizeof(url_parts_t));
	url->added_ts = inurl->added_ts;
	return url;
}

void delete_url(void *_url)
{
	url_t *url = (url_t *) _url;

	free(url->url);
	free(url->parts);
	free(url);
}

int print_url(void *_url) {
	url_t *url = (url_t *) _url;
	printf("%s, %lu, %s\n", url->parts->host, url->added_ts, url->url);
	return 0;
}

