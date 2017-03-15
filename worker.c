#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include "worker.h"
#include "http.h"
#include "links.h"
#include "read_line.h"

#define MAX_REDIRECTS 5
int master = 1;
char *msgs[] = { 
	"0:",
	"1:",
	"2:",
	"3:",
	"4:",
	"5:",
	"6:",
};

int trim(char *buf) {
	int i = 0;
	int len = strlen(buf);
	do {
		if(buf[len-1] == '\n') {
			buf[len-1] = 0;
			i++;
			len--;
		}
	} while(len > 0);
	return i;
}
worker_t *worker_new()
{
	worker_t *w; 
	int sfd[2];
	if(!(w = malloc(sizeof(*w)))) {
		perror("malloc");
		return NULL;
	}   
	if(socketpair(PF_UNIX, SOCK_STREAM, 0, sfd) < 0) {
		perror("socketpair");
		return NULL;
	}   
	w->status = AVAILABLE;
	cbuf_t *cbuf = calloc(1, sizeof(cbuf_t));
	if(!cbuf) {
		perror("calloc");
		return NULL;
	}

	w->pid = fork();
	
	if(w->pid < 0) {
		close(sfd[0]);
		close(sfd[1]);
		perror("fork");
		return NULL;
	} 
	else if(w->pid == 0) {
		master = 0;
		close(sfd[0]);
		//w->sfd = sfd[1];
		cbuf->fd = sfd[1];
		w->cbuf = cbuf;
		w->pid = getpid();
		worker(w);
		exit(0);
	} 
	
	close(sfd[1]);
	//w->sfd = sfd[0];
	cbuf->fd = sfd[0];
	w->cbuf = cbuf;
	return w;
}
void worker_del(worker_t *w) {
	if(w->cbuf->fd) 
		close(w->cbuf->fd);
	w->cbuf->fd = 0;
	free(w->cbuf);
	free(w);
}
worker_status_t getType(char *str) 
{
	// for now, i will change it later if got more 
	// than 10 types
	worker_status_t type = str[0]-'0';	
	if(type < 0 || type > 10) {
		printf("Error in getting type: %s, %d", str, str[0]);
		return -1;
	}
	return type;
}


int sendMsg(cbuf_t *cbuf, worker_status_t type, char *str) 
{
	char msg[1024];
	memset(msg, 0, sizeof(msg));
	strcpy(msg, msgs[type]);
	if(str)
		strncat(msg, str, sizeof(msg) - strlen(msg) - 2);
	strcat(msg, "\n");
	msg[sizeof(msg)-1] = 0;
	return write(cbuf->fd, msg, strlen(msg));
}

// gets a link by calling http.c getLink
// but also solves redirects 
static void 
worker_getLink(worker_t *self, char *base_url) 
{
	char *url = strdup(base_url);
	int redirects = 0;
	request_t *req;
	response_t *res;
	
	do {
		printf("Getting link: %s\n", url);
		req = request_new();
		req->url = url;

		if(!(res = getLink(req))) {
			printf("child: cannot get %s\n", base_url);
			//sendMsg(self->cbuf, ABORTING, NULL);
			sendMsg(self->cbuf, FAIL_LINK, url);
			return;
		}
		if(res->status != 301 && res->status != 302) 
			break;

		url = strdup(res->redir_url);
		request_del(req);
		response_del(res);
		redirects++;

	} while(redirects < MAX_REDIRECTS);

	if(redirects > 0) {
		printf("redirects: %d from original %s to %s\n", 
				redirects, base_url, url);
	}

	printf("got %d, body bytes: %lu\n", res->status, res->length);

	if(res->status == 200) {
		char **links;
		int i = 0;
	   	if((links = extract_links(url, res->body))) {
			while(links[i]) {
				//printf("SENDING LINK [%s]\n", links[i]);
				sendMsg(self->cbuf, PUT_LINK, links[i]);
				free(links[i]);
				i++;
			}
			free(links);
		}
	} 
	request_del(req);
	response_del(res);

	sendMsg(self->cbuf, GOT_LINK, base_url);
	if(redirects) sendMsg(self->cbuf, GOT_LINK, url);
	printf("child: sent ready\n");
}

void worker(worker_t *self) 
{
	char buf[512], *msg;
	int n, type;
	while(1) {
		// read message from master //
		if((n = read_line(self->cbuf, buf, sizeof(buf))) < 0) {
			fprintf(stderr, "child cannot read line\n");
			close(self->cbuf->fd);
			exit(1);
		} else if(n == 0) {
			fprintf(stderr, "master closed connection\n");
			close(self->cbuf->fd);
			exit(1);
		}
		buf[n] = 0;
		if(buf[n-1] == '\n') buf[n-1] = 0;
		type = getType(buf);
		if(type < 0) {
			fprintf(stderr, "Child: unknown type: %d, %s\n", type, buf);
			exit(1);
		}
		msg = buf+strlen(msgs[type]);
		if(type == ABORT) {
			sendMsg(self->cbuf, ABORTING, NULL);
			close(self->cbuf->fd);
			exit(0);
		}
		if(type == GET_LINK) {
			worker_getLink(self, msg);
		} else {
			printf("chld: unhandled type: %d\n", type);
		}
//		sleep(1);
	}
}
