#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include "worker.h"
#include "http.h"

char *msgs[] = { 
	"0:",
	"1:",
	"2:",
	"3:",
	"4:",
	"5:",
};

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
	w->pid = fork();
	if(w->pid < 0) {
		close(sfd[0]);
		close(sfd[1]);
		perror("fork");
		return NULL;
	} else if(w->pid == 0) {
		close(sfd[0]);
		w->sfd = sfd[1];
		w->pid = getpid();
		worker(w);
		exit(0);
	}   
	close(sfd[1]);
	w->sfd = sfd[0];
	return w;
}

worker_status_t getType(char *str) 
{
	// for now, i will change it later if got more 
	// than 10 types
	worker_status_t type = str[0]-'0';	
	return type;
}
// send message to parent/child
int sendMsg(int fd, worker_status_t type, char *str) 
{
	int n;
	char msg[4096];
	strcpy(msg, msgs[type]);
	if(str)
		strncat(msg, str, sizeof(msg)-strlen(msgs[type]-1));
	n = write(fd, msg, strlen(msg));
	return n;
}


void worker(worker_t *self) 
{
	char buf[4096], *msg;
	int n, type;
	response_t *res;
	while(1) {
		// read message from master //
		if((n = read(self->sfd, buf, sizeof(buf))) < 0) {
			perror("read");
			sendMsg(self->sfd, ABORTING, NULL);
			exit(0);
		}
		buf[n] = 0;
		type = getType(buf);
		msg = buf+strlen(msgs[type]);
		if(type == ABORT) {
			sendMsg(self->sfd, ABORTING, NULL);
			exit(0);
		}
		if(type == GET_LINK) {
			printf("Getting link: %s\n", msg);
			request_t *req;
			req = request_new();
			req->url = strdup(msg);
			if(!(res = getLink(req))) {
				request_del(req);
				response_del(res);
				printf("cannot get %s\n", msg);
				sendMsg(self->sfd, ABORTING, NULL);
				exit(0);
			}
			request_del(req);
			response_del(res);
			printf("got %d, body bytes: %lu\n", res->status, res->length);
			sendMsg(self->sfd, GOT_LINK, msg);
		}
		sleep(3);
	}
}
