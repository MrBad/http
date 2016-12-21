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
#include <fcntl.h>
#include <sys/wait.h>

typedef struct spider_params_t {
	list_t *queue;
	list_t *queue_fetched;
} spider_params_t;

spider_params_t *spider;



// get [limit] urls from queue, with distinct hostnames, 
// skipping and removing the links that are allready in spider->queue_fetched //
// returns a list on [limit] url_t linked list, that needs to be freed
list_t *queue_get_next_urls(int limit) {
	node_t *n, *h;
	int i = 0;
	list_t *ret;

	ret	= list_open(delete_url);
	for(n = spider->queue->head; n && i < limit; n=n->next) {
		// check if this host allready exists in returned list
		int found = 0;
		for(h=ret->head; h; h=h->next) {
			if(strcmp(((url_t *)n->data)->parts->host, ((url_t *)h->data)->parts->host) == 0) {
				printf("link %s already in ret\n", ((url_t*)n->data)->url);
				found = 1; 
				break;
			}
		}
		if(!found) {
			// check if this url is allready fetched;
			for(h = spider->queue_fetched->head; h; h=h->next) {
				if(strcmp(((url_t*)n->data)->url, ((url_t*)h->data)->url) == 0) {
					printf("link %s already fetched\n", ((url_t*)n->data)->url);
					list_del(spider->queue, n);
					found = 1;
					break;
				}
			}
		}
		if(!found) {
			i++;
			// add node to return list
			list_add(ret, url_clone((url_t *)n->data));
			// add the link to fetched list
			list_add(spider->queue_fetched, new_str(((url_t *)n->data)->url));
			// delete the node and free it from main queue list
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

char *getHost(char *host)
{
	struct hostent *h;
	h = gethostbyname(host);
	return inet_ntoa(*(struct in_addr*)h->h_addr);
}


void spider_loop() 
{

	//list_for_each(queue, print_url);
	list_t *next_list;
	node_t *n;
#define NUM_PROCS 10

	while(1) {
		next_list = queue_get_next_urls(NUM_PROCS);
		if(next_list->num_items == 0) {
			printf("Empty list\n");
			return;
		}

		int pipes[NUM_PROCS][2];
		unsigned int i = 0;

		for(n = next_list->head; n; n = n->next) {
			url_t *url = (url_t *) n->data;
			pid_t pid;
			if(pipe(pipes[i]) != 0) {
				perror("pipe");
				exit(1);
			}
			if((pid = fork()) < 0) {
				perror("fork()");
				exit(1);
			}
			if(pid == 0) {
				close(pipes[i][0]); // child close read
				char msg[512];
				snprintf(msg, sizeof(msg), "%s,%s", url->parts->host, getHost(url->parts->host));
				write(pipes[i][1], msg, sizeof(msg)); // write to parent / pipe
				exit(0);			
			} 
			close(pipes[i][1]); // parent close write
			i++;
		}


		for(i = 0; i < next_list->num_items; i++) {
			pid_t cpid;
			if((cpid = waitpid(-1, NULL, 0)) < 0) {
				perror("waitpid");
				exit(0);
			}
			char msg[512], host[256], ip[16];
			char *p = NULL;
			read(pipes[i][0], msg, sizeof(msg)); // read from child / pipe 
			if((p = strchr(msg, ',')) == NULL) {
				printf("cannot find , \n");
				continue;
			}
			strncpy(host, msg, p-msg);
			host[p-msg] = 0;
			strncpy(ip, ++p, sizeof(ip));
			// serch on the list and add IP
			for(n=next_list->head; n; n=n->next) {
				if(strcmp(((url_t *)n->data)->parts->host, host)==0) {
					strncpy(((url_t *)n->data)->ip, ip, sizeof(ip));
					break;
				}
			}
		}
		// print list //
		for(n=next_list->head; n; n=n->next) {
			printf("> %s, %s\n", ((url_t *)n->data)->parts->host, ((url_t *)n->data)->ip);
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
