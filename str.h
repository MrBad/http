#ifndef _STR_H
#define _STR_H


typedef struct {
	char *str;
	size_t len;
	size_t size;
} str_t;


str_t *strNew(char *str);
void strDel(void *str);
str_t *strDup(str_t *src);
char *strCat(str_t *dst, str_t *src);
char *strCpy(str_t *dst, str_t *src);
char *strAppend(str_t *str, char *s);
int  strCmp(str_t *s1, str_t *s2);

#endif
