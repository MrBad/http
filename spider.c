#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <assert.h>
#include <signal.h>
#include <sys/wait.h>
#include "list.h"
#include "tree.h"
#include "str.h"
#include "url.h"
#include "worker.h"
#include "read_line.h"

extern int master;

#define MAX_THREADS 10

typedef struct spider_params 
{
	list_t *queue;	// list of url_t
	tree_t *seen;	// list of str_t
	char *seen_db_file;
	char *queue_db_file;
	worker_t *workers[MAX_THREADS];
} spider_t;


static int min_len;

#define min(a, b) (a) < (b) ? a : b

int load_seen_db(spider_t *spider) 
{
	FILE *fp;
	char buf[4096];
	int len, i = 0;
	printf("Loading seen data from %s", spider->seen_db_file);
	fflush(stdout);
	if(!(fp = fopen(spider->seen_db_file, "r"))) {
		fprintf(stdout, " empty, starting from 0\n");
		return 0;
	}
	while(fgets(buf, sizeof(buf), fp)) {
		len = strlen(buf);
		buf[len-1] = 0;
		min_len = min(min_len, len);
		tree_add(spider->seen, strNew(buf));
		i++;
	}
	printf(", %d urls\n", i);
	fclose(fp);
	return 0;
}

int save_url(void *url, void *ctx) 
{
	int *fd = ctx, n = 0;
	str_t *s = url;
	if(!url) return -1;
	n = write(*fd, s->str, s->len);
	n += write(*fd, "\n", 1);
	return n;
}

int save_seen_db(spider_t *spider) 
{
	void *ctx;
	int fd = open(spider->seen_db_file, O_WRONLY|O_TRUNC|O_CREAT, 0666);
	if(fd < 0) {
		perror("open");
		return -1;
	}
	ctx = &fd;
	tree_foreach(spider->seen, save_url, 0, ctx);
	close(fd);
	return 0;
}

int load_queue_db(struct spider_params *spider) 
{
	FILE *fp;
	char buf[4096];
	int len, i = 0;
	url_t *url;
	str_t *str;
	printf("Loading queue from file %s", spider->queue_db_file);	
	fflush(stdout);
	if(!(fp = fopen(spider->queue_db_file, "r"))) {
		fprintf(stderr, ". Cannot open initial queue file, aborting\n");
		return -1;
	}
	while(fgets(buf, sizeof(buf), fp)) {
		len = strlen(buf);
		buf[len-1] = 0;
		str = strNew(buf);
		if(!str) continue;
		url = url_new(str);
		if(!url) {
			strDel(str);
			continue;
		}
		list_add(spider->queue, url);
		i++;
	}
	printf(", %d urls\n", i);
	if(i == 0) {
		fprintf(stderr, "Nothing more to fetch\n");
		return -1;
	}
	return 0;
}

int save_queue_db(spider_t *spider) 
{
	int fd = open(spider->queue_db_file, O_WRONLY|O_TRUNC|O_CREAT, 0666);
	if(fd < 0) {
		perror("open");
		return -1;
	}
	list_node_t *n;
	list_foreach(spider->queue, n) {
		url_t *u = n->data;
		if(write(fd, u->url->str, u->url->len) < 0) {
			perror("write");
			return -1;
		}
		write(fd, "\n", 1);
	}
	close(fd);
	return 0;
}


void del_url(void *data) 
{
	url_del((url_t*) data);
}

int cmp_str(void *s1, void *s2) 
{
	str_t *a, *b;
	a = s1;
	b = s2;
	return strcmp(a->str, b->str);
	return strCmp((str_t*)s1, (str_t*)s2);
}

void del_str(void *data) 
{
	strDel((str_t*)data);
}

int create_queues(struct spider_params *spider) 
{
	if(!(spider->queue = list_open(del_url))) {
		return -1;
	}
	if(!(spider->seen = tree_open(cmp_str, del_str))) {
		return -1;
	}
	// read dbs //
	if(load_seen_db(spider) < 0) {
		return -1;
	}
	if(load_queue_db(spider) < 0) {
		return -1;
	}
	return 0;
}

int close_queues(struct spider_params *spider) 
{
	save_seen_db(spider);
	save_queue_db(spider);
	tree_close(spider->seen);
	list_close(spider->queue);
	return 0;
}


url_t *url_arr[MAX_THREADS];

// don't hit too fast same host! //
char **seen_hosts = NULL;
int num_sh = 0, size_sh = 0;


void add_seen_host(char *host) 
{
	int i;
	if(size_sh == 0) {
		size_sh = 2;
		seen_hosts = calloc(size_sh, sizeof(char *));
	}
	seen_hosts[num_sh++] = strdup(host);
	if(num_sh == size_sh) {
		size_sh *= 2;
		seen_hosts = realloc(seen_hosts, size_sh * sizeof(char *));
		if(!seen_hosts) exit(1);
		for(i = num_sh; i < size_sh; i++) {
			seen_hosts[i] = 0;
		}
#if 0
		FILE *fp = fopen("hosts.db", "a");
		fprintf(fp, "-----------\nnum_sh: %d, size_sh: %d\n----------\n", num_sh, size_sh);
		for(i = 0; i < num_sh; i++) {
			fputs(seen_hosts[i], fp);
			fputs("\n", fp);
		}
		fclose(fp);
#endif
	}
	if(num_sh > 1000) {
		for(i = 0; i < num_sh; i++) {
			seen_hosts[i] = seen_hosts[i+1];
		}
		num_sh--;
	}
}

int del_seen_hosts() {
	int i;
	if(!seen_hosts)
		return -1;
	for(i = 0; i < num_sh; i++)
	   	free(seen_hosts[i]);
	num_sh = 0;
	return 0;
}
int in_host_arr(char *host) {
	int i;
	if(!seen_hosts)
		return -1;
	for(i = 0; i < num_sh; i++) {
		if(strcmp(host, seen_hosts[i]) == 0)
			return 0;
	}
	return -1;
}

#if 0
int in_host_arr(char *host) {
	int i;
	for(i = 0; i < MAX_THREADS; i++) {
		if(!url_arr[i]) continue;
		if(strcmp(host, url_arr[i]->parts->host) == 0) {
			return 0;
		}
	}
	return -1;
}
#endif

int getNextUrls(spider_t *spider) 
{
	list_node_t *node;
	int i = 0, items = 0;	
	url_t *url;
	list_node_t *nodes[MAX_THREADS];
	if(!spider)
		exit(1);
	memset(url_arr, 0, sizeof(url_arr));
	memset(nodes, 0, sizeof(nodes));
	list_foreach(spider->queue, node) {
		url = node->data;
		if(in_host_arr(url->parts->host) < 0) {
			nodes[i] = node;
			url_arr[i++] = url_dup(url);
			items++;
			add_seen_host(url->parts->host);
		}
		if(i == MAX_THREADS) break;
	}

	for(i = 0; i < MAX_THREADS; i++) {
		if(!nodes[i]) continue;
		list_del(spider->queue, nodes[i]);
	}
	if(items == 0) {
		printf("SEEN ALL HOSTS\n\n\n");
		exit(1);
		del_seen_hosts();
		return getNextUrls(spider);
	}
	return items;
}

int getFreeWorker(spider_t *spider) 
{
	int i;
	worker_t *w;
	for(i = 0; i < MAX_THREADS; i++) {
		if(!spider->workers[i]) {
			if(!(w = worker_new())) {
				return -1;
			}
			spider->workers[i] = w;
			break;
		} else if(spider->workers[i]->status == AVAILABLE) {
			break;
		}
	}
	if(i == MAX_THREADS) {
		return -1;
	}
	return i;
}

void 
spider_exit(spider_t *spider, int status) 
{	
	int i;
	for(i = 0; i < MAX_THREADS; i++) {
		if(!spider->workers[i])
			continue;
		sendMsg(spider->workers[i]->cbuf, ABORT, NULL);
	}
	
	close_queues(spider);
	exit(status);
}

void 
processWorkers(spider_t *spider) 
{
	char buf[4096];
	struct timeval tv;
	fd_set rfds;
	int i, n, maxfd = 0, ret;

	tv.tv_sec = 3;
	tv.tv_usec = 0;
	
	FD_ZERO(&rfds);
	for(i = 0; i < MAX_THREADS; i++) {
		if(!spider->workers[i]) 
			continue;
		FD_SET(spider->workers[i]->cbuf->fd, &rfds);
		maxfd = spider->workers[i]->cbuf->fd > maxfd ? 
			spider->workers[i]->cbuf->fd : maxfd;
	}

	if((ret = select(maxfd+1, &rfds, NULL, NULL, &tv)) < 0) {
		perror("select");
		spider_exit(spider, 1);
	} else if (ret > 0) {
		for(i = 0; i < MAX_THREADS; i++) {
			if(!spider->workers[i]) 
				continue;
			if(FD_ISSET(spider->workers[i]->cbuf->fd, &rfds)) {
				worker_t *worker = spider->workers[i];
eagain:
				if((n = read_line(worker->cbuf, buf, sizeof(buf))) < 0) {
					fprintf(stderr, "MASTER: cannot read line\n");
					worker_del(worker);
					spider->workers[i] = 0;
				} else if(n == 0) {
					continue;
					fprintf(stderr, "MASTER: child has close connection\n");
					// close child //
					worker_del(worker);
					spider->workers[i] = 0;
				}
					
				if(buf[n-1] == '\n') buf[n-1] = 0;
				int type = getType(buf);
				char *msg = buf+2;


				if(type == GOT_LINK) {
FILE *fp = fopen("tadd.db", "a");fputs(msg, fp); fputs("\n", fp);fclose(fp);
					// mark worker AVAILABLE
					// and add link to seen links
					printf("worker is ready\n");
					worker->status = AVAILABLE;
					str_t *str = strNew(msg);
					if(tree_find(spider->seen, str)==0) {
						strDel(str);
						continue;
					}
					tree_add(spider->seen, str);
				}
				else if(type == FAIL_LINK) {
					printf("fail link %s\n", msg);
					worker->status = AVAILABLE;
				}
				else if(type == ABORTING) {
					// worker is aborting //
					// free it and mark slot free //
					// later, maibe i will just set pid 0 //
					printf("MASTER: child requested abort\n");
					worker_del(worker);
					//kill(spider->workers[i]->pid, SIGKILL);
					spider->workers[i] = 0;
				} 

				// worker discovered a new url //
				else if(type == PUT_LINK) {
					str_t *str = strNew(msg);
					if(!str) {
						perror("strNew");
						goto eagain;
					}
					if(tree_find(spider->seen, str)==0) {
						//printf("url %s exists\n", str->str);
						strDel(str);
						goto eagain;
					}
					//tree_add(spider->seen, str);
					url_t *url = url_new(str);
					if(!url) {
						strDel(str);
						perror("url_new");
						goto eagain;
					}
					list_add(spider->queue, url);
					//printf("got link: [%s]\n", msg);
					goto eagain;
				}

				// worker is sending page txt //
				else if(type == PUT_TXT) {
					printf("handling txt\n");
				} 
				else {
					printf("Master: Not handled %d [%s]\n", type, msg);
			//		spider_exit(spider, 1);
				}
			}
		}	
	} else {
		write(1, ".", 1);
	}
	
}

void 
spider_loop(spider_t *spider) 
{
	int i, id ;
	static int iterations = 0;
	for(;;) {
		getNextUrls(spider);
		if(spider->queue->items == 0) {
			printf("Queue starving\n");
			spider_exit(spider, 2);
		}
		for(i = 0; i < MAX_THREADS; i++) {
			if(!url_arr[i]) continue;
			while((id = getFreeWorker(spider)) < 0) {
				processWorkers(spider);
			}
			// transmit worker the url //
			sendMsg(spider->workers[id]->cbuf, GET_LINK, url_arr[i]->url->str);
			spider->workers[id]->status = FETCHING;
		}
		if(iterations++ == 10000) {
			printf("Reached %d iterations\n", iterations);
			break;
		}
	}
}

spider_t *sp;
void onExit() 
{
	if(!master) 
		return;
	sigset_t mask_set, old_mask;
	signal(SIGTERM, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	sigfillset(&mask_set);
	sigprocmask(SIG_SETMASK, &mask_set, &old_mask);


	printf("Receiving terminate signal\n");
	fflush(stdout);

	spider_exit(sp, 0);	

	printf("Spider exited normally\n"); fflush(stdout);
	kill(getpid(), SIGTERM);
}

int main() 
{
	struct spider_params *spider;
	char *queue_db_file = "queue.db";
	char *seend_db_file = "seen.db";
	int i;
	//signal(SIGPIPE, SIG_IGN);
//	signal(SIGCHLD, SIG_IGN);
	if(!(spider = malloc(sizeof(struct spider_params)))) {
		perror("malloc");
		return 1;
	}
	sp = spider;
	spider->queue_db_file = queue_db_file;
	spider->seen_db_file = seend_db_file;
	for(i = 0; i < MAX_THREADS; i++)
		spider->workers[i] = NULL;
	
	if(create_queues(spider) < 0) {
		fprintf(stderr, "Cannot create queues\n");
		return 1;
	}

	signal(SIGTERM, onExit);
	signal(SIGINT, onExit);

	spider_loop(spider);
	spider_exit(spider, 0);
}
