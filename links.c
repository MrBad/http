#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "links.h"
#include "parse_url.h"

char *make_abs_url(char *base, char *rel) 
{
	char *ret, *p;
	if(strncmp(rel, "http://", 7) == 0) {
		return strdup(rel);
	} else if(strncmp(rel, "https://", 8) == 0) {
		return strdup(rel);
	}
	ret = calloc(1, strlen(base) + strlen(rel) + 2);
	if(*rel == '/') {
		if(!(p = strstr(base, "://")))
		return NULL;
		p += 3; 
		while(*p && *p != '/') p++;
		strncpy(ret, base, p - base);
		strcat(ret, rel);
	} else {
		strcat(ret, base);
		strcat(ret, rel);
	}
	// TODO  solve ./ and ../
	return ret;
}

char **extract_links(char *base_link, char *str) 
{	
	char **arr, *p = str, *k, *n, buf[512], *link;
	int size = 16, elmns = 0, i;
	if(!base_link || !str)
		return NULL;

	if(!(arr = calloc(size, sizeof(char *)))) {
		perror("calloc");
		return NULL;
	}
	while((p = strstr(p, "<a "))) {
		p += 3; i = 0;
		while(*p && *p != '>') {
			if(i < 512) buf[i++] = *p;
			p++;
		}
		if(!*p) break;
		buf[i] = 0;
		if((k = strstr(buf, "href"))) {
			while(*k && *k!='"') k++;
			if(!*k) break;
			k++;
			n = k;
			while(*k && *k!='"') k++;
			if(!*k) break;
			strncpy(buf, n, k-n);
			buf[k-n] = 0;
			if(!(link = make_abs_url(base_link, buf))) 
				continue;	
			arr[elmns++] = link;
			if(elmns > size - 1) {
				size *= 2;
				if(!(arr = realloc(arr, size * sizeof(char *))))
					return NULL;
				for(i = elmns; i < size; i++)
					arr[i] = NULL;
			}
		}
	}
	return arr;
}
