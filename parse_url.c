#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#include "parse_url.h"

//TODO: user/pass - what's the f. sintax, because i do not remember it...
//		free url_parts_t struct on error... more testing pls 
//		No Regexp, no sscanf :)

struct url_parts_t* parse_url(char *url) {

	char *p, *n;
	int port;
	struct url_parts_t *url_parts;

	if(! (p = strstr(url, "://"))) {
		printf("Invalid scheme in url - use http://...\n");
		return NULL;
	}

	url_parts = (struct url_parts_t *) calloc(1, sizeof(struct url_parts_t));
	if(url_parts == NULL) {
		perror("Out of memory\n");
		return NULL;
	}
	url_parts->scheme = strndup(url, p-url);

	p += 3;
	n = p;

	for(;*n && *n != ':' && *n != '/' && *n != '?' && *n != '#'; n++); // until end, or : or / or ? or #
	if(n == p) {
		printf("Invalid host - empty host\n");
		free(url_parts);
		return NULL;
	}

	url_parts->host = strndup(p, n - p);

	if(*n == ':') {
		for(p = ++n, port = 0; *n && *n != '/' && *n != '?' && *n != '#'; n++) {
			if(*n < '0' || *n > '9') {
				free(url_parts->host);
				free(url_parts);
				printf("Invalid port - should contain only digits\n");
				return NULL;
			}
			port = port * 10 + *n - '0';
			if(port > 65535 || port == 0) {
				free(url_parts->host);
				free(url_parts);
				printf("Invalid port - should be between 0 and 65535\n");
				return NULL;
			}
		}
		url_parts->port = (unsigned short) port;
	}

	if(*n == '/') {
		for(p = n; *n && *n != '?' && *n != '#'; n++); // until end, or ? or #
		url_parts->path = strndup(p, n - p);
		p = n;
	}
	if(*n == '?') {
		for(p = ++n; *n && *n != '#'; n++);
		url_parts->query_string = strndup(p, n - p);
	}
	if(*n == '#') {
		for(p = ++n; *n; n++);
		url_parts->fragment = strndup(p, n - p);
	}

	if(!url_parts->port) {
		//struct servent *serv;
		//serv = getservbyname(url_parts->scheme, "tcp");
		//printf("[%i]\n", serv->s_port);
		if(strcmp(url_parts->scheme, "http") == 0) {
			url_parts->port = 80;
		}
	}
	if(!url_parts->path) {
		url_parts->path = strdup("/");
	}
	return url_parts;
}

#if 0
char *make_url(struct url_parts_t url_parts) {
	
}
#endif


void free_url_parts(struct url_parts_t *url_parts) {
	free(url_parts->scheme);
	free(url_parts->host);
	free(url_parts->path);
	if(url_parts->user) {
		free(url_parts->user);	
	}
	if(url_parts->pass) {
		free(url_parts->pass);
	}
	if(url_parts->query_string) {
		free(url_parts->query_string);
	}
	if(url_parts->fragment) {
		free(url_parts->fragment);
	}
	free(url_parts);
}

