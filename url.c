#include <stdio.h>
#include "parse_url.h"

int main(int argc, char **argv) {

	if(argc < 2) {
		printf("Usage: %s [url]\n", argv[0]);
		return 1;
	}

	struct url_parts_t *url_parts;
	url_parts = parse_url(argv[1]);
	if(!url_parts) {
		return 1;
	}
	printf("scheme: [%s], host: [%s], port: [%d], path: [%s], query_string: [%s], fragment: [%s]\n", 
			url_parts->scheme, url_parts->host, url_parts->port, url_parts->path, url_parts->query_string, url_parts->fragment);

	return 0;
}

