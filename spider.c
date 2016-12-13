#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <assert.h>
#include "demonize.h"
#include "list.h"
#include "spider.h"
#include "parse_url.h"
#include "init_urls.h"
#include "url.h"
#include "str.h"

typedef struct spider_params_t {
	list_t *queue;
	list_t *queue_fetched;
} spider_params_t;

spider_params_t *spider;



// get [limit] urls from queue, with distinct hostnames //
list_t *queue_get_next_urls(int limit) {
	node_t *n, *h;
	int i = 0;
	list_t *ret;
	
	ret	= list_open(delete_url);
	for(n = spider->queue->head; n && i < limit; n=n->next) {
		// check if this host allready exists in ret
		int found = 0;
		for(h=ret->head; h; h=h->next) {
			if(strcmp(((url_t *)h->data)->parts->host, ((url_t *)n->data)->parts->host) == 0) {
				printf("link %s already in ret\n", ((url_t*)n->data)->url);
				found = 1; 
				break;
			}
		}
		if(!found) {
			// check if this url is allready fetched;
			for(h = spider->queue_fetched->head; h; h=h->next) {
				if(strcmp(((url_t*)h->data)->url, ((url_t*)n->data)->url) == 0) {
					printf("link %s already fetched\n", ((url_t*)n->data)->url);
					list_del(spider->queue, n);
					found = 1;
					break;
				}
			}
		}
		if(!found) {
			i++;
			list_add(ret, url_clone((url_t *)n->data));
			list_add(spider->queue_fetched, new_str(((url_t *)n->data)->url));
			list_del(spider->queue, n);
		}
	}
	return ret;
}


void create_initial_queues()
{
	
	spider->queue = list_open(delete_url);
	spider->queue_fetched = list_open(delete_str);

	int urls_size = sizeof(initial_urls) / sizeof(*initial_urls);
	for(int i = 0; i < urls_size; i++) {
		list_add(spider->queue, new_url(initial_urls[i]));
	}
//	list_add(spider->queue_fetched, new_str("http://www.microsoft.com"));
}

void close_queues() {
	list_close(spider->queue);
	list_close(spider->queue_fetched);
}


void spider_loop() 
{
	
	//list_for_each(queue, print_url);
	list_t *next_list;
	node_t *n;

	while(1) {
		next_list = queue_get_next_urls(10);
		assert(next_list != NULL);
		if(next_list->num_items == 0) {
			printf("Empty list\n");
			return;
		}
		for(n = next_list->head; n; n = n->next) {
			struct hostent *host;
			host = gethostbyname(((url_t *)n->data)->parts->host);
			if(host == NULL) {
				list_del(next_list, n);	
			}
			printf(">%s - %s\n", inet_ntoa(*(struct in_addr*)host->h_addr), ((url_t *)n->data)->parts->host);
		}
		list_close(next_list);
	}
}

int main(void) 
{
	
	if(DEMONIZE) {
		if(! demonize(PIDFILE)) {
			printf("Cannot demonize, exiting");
			exit(1);
		}
	}
	spider = (spider_params_t *) malloc(sizeof(spider_params_t));
	if(!spider) {
		perror("spider");
		return 1;
	}
	create_initial_queues();
	
	spider_loop();

	node_t *n;
	for(n=spider->queue_fetched->head; n; n=n->next) {
		//printf("%s\n", ((str_t *)n->data)->str);
	}
	printf("fetched: %d\n", spider->queue_fetched->num_items);
	close_queues();
	
}
