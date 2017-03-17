#ifndef _HTTP_H
#define _HTTP_H
#include "str.h"

typedef struct 
{
	char *header;
	char *body;
	int status;
	char *redir_url;	// if redirected, here will be url
	ssize_t length;
	char *type, *subtype;
	char *last_url;
	char *charset;
	int redirects;
	int isChunked;
} response_t;

typedef struct {
	char *header;
	char *url;
	char *user_agent;
	int allow_redirect;
	int redirects;
	int max_redirects;
} request_t;

#define DEFAULT_USER_AGENT "Mozilla/5.0 (compatible; RoBot/0.1;)"

response_t *getLink(request_t *req);

request_t *request_new();
void request_del(request_t *req);
void response_del(response_t *res);

#endif
