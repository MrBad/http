#ifndef _WORKER_H
#define _WORKER_H

typedef enum {
	ABORT,      // child should exit
	ABORTING,   // child got error
	GET_LINK,   // child should get link
	GOT_LINK,   // child got link
	PUT_LINK,   // master should add the link to queue
	PUT_TXT,    // master should do something with text
} msg_t;

extern char *msgs[];

typedef enum { 
	AVAILABLE,  
	RES_HOST,       // resolving hostname
	FETCHING,       // getting webpage
	PROCESSING,     // processing webpage
} worker_status_t;


typedef struct {
	int sfd;
	int pid;
	worker_status_t status;
} worker_t;


void worker(worker_t *self);
worker_t *worker_new();
worker_status_t getType(char *str);
int sendMsg(int fd, worker_status_t type, char *str);
#endif
