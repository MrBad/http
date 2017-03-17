#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "parse_url.h"

char *allowedSchemes[] = {
	"http",
	"https"
};

#define arrLen(x) sizeof(x) / sizeof(*x)
bool isSchemeAllowed(char *scheme) {
	unsigned int i;
	for (i = 0; i < arrLen(allowedSchemes); i++) {
		if(strcasecmp(scheme, allowedSchemes[i]) == 0) {
			return true;
		}
	}
	return false;
}
char *mkabsurl(char *base, char *rel)
{
	url_parts_t *parts;
	char *url = NULL, *p;
	size_t len, l2;
	if(strstr(rel, "://")) {
		if(!(parts = parse_url(rel)))
			return NULL;
		if(!isSchemeAllowed(parts->scheme)) {
			free_url_parts(parts);
			return NULL;
		}
		if((p = strchr(rel, '#')))
			len = p - rel;
		else
			len = strlen(rel);
		url = malloc(len + 1);
		memcpy(url, rel, len);
		url[len] = 0;

	} else {
		if(!(parts = parse_url(base)))
			return NULL;
		if(!isSchemeAllowed(parts->scheme)) {
			free_url_parts(parts);
			return NULL;
		}
		len = strlen(base) + strlen(rel);
		url = malloc(len + 2);
		snprintf(url, len, "%s://%s", parts->scheme, parts->host);
		if (parts->port != 80) {
			l2 = strlen(url);
			snprintf(url, len - l2, ":%d", parts->port);
		}
		if(rel[0] == '/') {
			if((p = strchr(rel, '#')))
				l2 = p - rel;
			else
				l2 = strlen(rel);
			assert(strlen(url)+l2 <= len);
			strncat(url, rel, l2);
		}
		else {
			p = strrchr(parts->path, '/');
			if(strchr(p, '.'))
				strncat(url, parts->path, p-parts->path+1);
			else
				strcat(url, parts->path);
			if(url[strlen(url)-1] != '/')
				strcat(url, "/");
			strcat(url, rel);
		}
	}
	free_url_parts(parts);
	return url;
}
