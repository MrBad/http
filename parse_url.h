#ifndef _PARSE_URL_
#define _PARSE_URL_

typedef struct url_parts_t {
	char *scheme;
	char *user;
	char *pass;
	char *host;
	unsigned short port;
	char *path;
	char *query_string;
	char *fragment;
} url_parts_t;

url_parts_t* parse_url(char *url);
void free_url_parts(url_parts_t *url_parts);
url_parts_t *url_parts_dup(url_parts_t *p);
#endif
