#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "str.h"

str_t *new_str(char *str) {
	str_t *s;
	s = (str_t *) malloc(sizeof(str_t));
	if(!s) {
		perror("malloc");
		return NULL;
	}

	s->str = strdup(str);
	s->length = strlen(s->str);
	s->size = s->length;
	return s;
}

void delete_str(void *_s) {
	str_t *s = (str_t *)_s;
	free(s->str);
	free(s);	
}
