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
url_t *queue_get_next_url(){
	node_t *n = NULL;
	if((n = spider->queue->head)) {
		list_add(spider->queue_fetched, new_str(((url_t*)n->data)->url) );
		spider->queue->head = n->next;
		return ((url_t *)n->data);	
	}
	return NULL;
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

struct pid_fds_t {
	pid_t pid;
	char host[256];
	int fd[2];	
};

#define MAX_PROCS 30
void spider_loop() {
	url_t *url;
	int num_procs = 0;
	pid_t pid;
	struct pid_fds_t pid_fds[MAX_PROCS];
	int i,found;
	char msg[512];
	int solved = 0;
	for(i=0;i<MAX_PROCS;i++){
		pid_fds[i].pid = 0;
	}
	for(;;) {
		url = queue_get_next_url();
		if(url) {
			found = 0;
			for(i=0; i < MAX_PROCS; i++) {
				if(pid_fds[i].pid == 0) {
					found = 1; break;
				}
			}
			assert(found != 0);
			if(pipe(pid_fds[i].fd)!=0){
				perror("pipe");exit(EXIT_FAILURE);
			}
			strncpy(pid_fds[i].host, url->parts->host, sizeof(pid_fds[i].host));
			if((pid = fork()) < 0) {
				perror("fork");
				exit(EXIT_FAILURE);
			}
			if(pid == 0) {
				close(pid_fds[i].fd[0]);
				snprintf(msg, sizeof(msg), "%s", getHost(url->parts->host));
				write(pid_fds[i].fd[1], msg, strlen(msg)+1);
				//close(pid_fds[i].fd[1]); // will be closed by exit?!
				exit(EXIT_SUCCESS);	
			}
			close(pid_fds[i].fd[1]);
			pid_fds[i].pid = pid;
		}

		if(++num_procs >= MAX_PROCS || !url) {
			pid = waitpid(-1, NULL, 0);
			if(pid < 0) {
				printf("No more childs!\n");
				break;
			}
			for(i = 0, found = 0; i < MAX_PROCS; i++) {
				if(pid_fds[i].pid == pid) {
					found = 1; break;
				}
			} 
			assert(found != 0);
			read(pid_fds[i].fd[0], msg, sizeof(msg));
			printf("host: %s, ip: %s\n", pid_fds[i].host, msg);
			pid_fds[i].pid=0;
			close(pid_fds[i].fd[0]);
			num_procs--;
			solved++;
		}
	}
	printf("solved: %d\n",solved);
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
