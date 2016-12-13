#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "demonize.h"
#include "list.h"
#include "spider.h"
#include "parse_url.h"
#include "init_urls.h"

typedef struct url_t {
	char url[256];
	char domain[64];
	unsigned int added_ts;
} url_t;


void delete_url(void *url) {
//	printf("Freeing %s\n", ((url_t *)url)->url);
	free(url);	
}

url_t *new_url(char *str) {
	url_t *url = (url_t *) malloc(sizeof(url_t));
	strncpy(url->url, str, sizeof(url->url)-1);
	url_parts_t *parts;
	parts = parse_url(str);
	strncpy(url->domain, parts->host, sizeof(url->domain)-1);
	free_url_parts(parts);
	return url;
}

int print_url(node_t *node) {
	url_t *url = (url_t *) node->data;
	printf("%s, %s\n", url->domain, url->url);
	return 0;
}

int main(void) {
	list_t *queue, *queue_seen;
	unsigned int urls_size, i;
	url_t *url;

	if(DEMONIZE) {
		if(! demonize(PIDFILE)) {
			printf("Cannot demonize, exiting");
			exit(1);
		}
	}

	queue = list_open(delete_url);
	queue_seen = list_open(NULL);

	//printf("%i\n", sizeof (initial_urls) / sizeof(*initial_urls));
	urls_size = sizeof(initial_urls) / sizeof(*initial_urls);
	for( i = 0; i < urls_size; i++) {
		url = new_url(initial_urls[i]);
		list_add(queue, url);	
	}

	list_for_each(queue, print_url);

	list_close(queue);
	list_close(queue_seen);
}
