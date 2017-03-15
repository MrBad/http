#include <stdio.h>
#include <string.h>
#include "http.h"
#include "links.h"

int main(int argc, char *argv[]) 
{
	response_t *res;
	request_t  *req;
	if(argc != 2) {
		fprintf(stderr, "usage: %s url\n", argv[0]);
		return 1;
	}

	req = request_new();
	req->url = strdup(argv[1]);
	req->allow_redirect = 1;
	req->max_redirects = 4;
	if(!(res = getLink(req))) {
		request_del(req);
		response_del(res);
		fprintf(stderr, "cannot get %s\n", argv[1]);
		return 1;
	}
	printf("req was: \n_____________\n");
	printf("%s", req->header);
	printf("___________HEADER\n");
	printf("%s", res->header);
	printf("___________\n");
	
	printf("status: %d\n", res->status);
	if(req->redirects) {
		printf("redir url: %s\n", req->url);
	}
	printf("charset: %s\n", res->charset);
	printf("Body Len: %lu\n", res->length);
	printf("Content-Type: %s/%s\n", res->type, res->subtype);

	extract_links(req->url, res->body);

	return 0;	
}
