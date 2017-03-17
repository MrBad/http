#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include "parse_url.h"


char *allowedSchemes[] = {
		"http",
		"https"
};

#define arrLen(x) sizeof(x) / sizeof(*x)
/*
static bool validScheme(char *url)
{
	char *p, scheme[16];
	size_t len;
	int i;
	if ((p = strstr(url, "://"))) {
		len = p - url;
		strncpy(scheme, url, len);
		scheme[len] = 0;
		for (i = 0; i < arrLen(allowedSchemes); i++)
			if (strcmp(scheme, allowedSchemes[i]) == 0)
				return true;
	}
	return false;
}

static bool validDomain(char *url)
{
	char *p;
	char domain[512];
	if((p = strstr(url, "://")))
		url = p + 3;
	int i;
	if(!isalnum(url[0]))
		return false;
	for(i = 0; i < sizeof(domain)-1; i++) {
		if(url[i] == '/' || url[i] == 0) break;
		if(url[i] == '.') {
			if(i == 0)
				return false;
			// prev and next should be a-z0-9
			if(!isalnum(url[i-1]) || !isalnum(url[i+1]))
				return false;
		}
		if(!isalnum(url[i]) && url[i]!='.' && url[i]!='-') {
			return false;
		}
		domain[i] = url[i];
	}
	domain[i] = 0;
	if(!(p = strrchr(domain, '.')))
		return false;
	p++;
	if((i - (p-domain)) < 2)
		return false;
	while(*p) {
		if(!isalnum(*p))
			return false;
		p++;
	}

	return true;
}
*/

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
	if(strstr(rel, "://") || strncmp(rel, "//", 2) == 0) {
		char *tmp = NULL;
		if (strncmp(rel, "//", 2) == 0) {
			size_t l = strlen("http:") + strlen(rel);
			tmp = malloc(l + 1);
			strcpy(tmp, "http:");
			strcat(tmp, rel);
			tmp[l] = 0;
			rel = tmp;
		}
		if (!(parts = parse_url(rel))) {
			if(tmp) free(tmp);
			return NULL;
		}
		if(!isSchemeAllowed(parts->scheme)) {
			if(tmp) free(tmp);
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
		if(tmp) free(tmp);
	} 
	else {
		// skip javascript:, tel:
		if(strchr(rel, ':')) {
			return NULL;
		}
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
			if((p = strchr(rel, '#')))
				l2 = p - rel;
			else
				l2 = strlen(rel);
			strncat(url, rel, l2);
		}
	}
	free_url_parts(parts);
	return url;
}

