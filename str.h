#ifndef _STR_H_
#define _STR_H_
#include <stdio.h>

typedef struct str_t {
	char *str;
	size_t length;
	size_t size;
} str_t;


str_t *new_str(char *str);
void delete_str(void *str);


#endif
