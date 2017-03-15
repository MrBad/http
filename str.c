#include <stdio.h>
#include <string.h>
#include <stdlib.h>
typedef struct {
	char *str;
	size_t len;
	size_t size;
} str_t;

str_t *strNew(char *str) 
{
	str_t *s;
	if(!(s = malloc(sizeof(str_t)))) {
		perror("malloc");
		return NULL;
	}
	s->len = str ? strlen(str) : 0;
	s->size = s->len+1;
	if(!(s->str = malloc(s->size))) {
		perror("malloc");
		free(s);
		return NULL;
	}
	if(str) strcpy(s->str, str);
	s->str[s->len] = 0;
	return s;
}

int bads;
void strDel(void *str) 
{
	str_t *s = str;
	if(!str) {
		bads++;
		printf("strDel: bptr %d", bads);
		return;
	}
	if(s->str) {
		free(s->str);
	}
	free(s);
}

static void strExpand(str_t *str, unsigned int len) 
{
	char *s;
	if(len <= str->size) {
		printf("str_expand: len < str->size\n");
	}
	if(!(s = realloc(str->str, len))) {
		perror("realloc");
		return;
	}
	str->str = s;
	str->len = len;
	str->size = len;
}

char *strCat(str_t *dst, str_t *src) 
{
	if(dst->size < dst->len + src->len + 1) {
		strExpand(dst, dst->len + src->len + 1);
	}
	return strcat(dst->str, src->str);
}

char *strCpy(str_t *dst, str_t *src) 
{	
	char *str;
	if(dst->size < src->len + 1) {
		strExpand(dst, src->len + 1);
	}
	str = strcpy(dst->str, src->str);
	dst->len = strlen(dst->str);
	return str;
}

str_t *strDup(str_t *src) 
{
	return strNew(src->str);
}

char *strAppend(str_t *str, char *s) 
{
	int len = strlen(s);
	if(str->size < str->len + len + 1) {
		strExpand(str, str->len + len + 1);
	}
	return strcat(str->str, s);
}

int strCmp(str_t *s1, str_t *s2) 
{
	if(s1->len != s2->len) {
		return s1->len - s2->len;
	}
	return strcmp(s1->str, s2->str);
}

#ifdef TEST
int main()
{
	str_t *s = strNew("Testing");
	str_t *s2 = strNew(" this thing \n");
	str_t *s3 = strNew(NULL);

	strCat(s, s2);
	strCpy(s3, s);
	strAppend(s3, "aaaaa");
	printf("%lu, %lu, %lu %s\n", s->len, s->size, strlen(s->str), s->str);
	printf("%lu, %lu, %lu %s\n", s3->len, s3->size, strlen(s3->str), s3->str);

	strDel(s);
	strDel(s2);
	strDel(s3);
	return 0;
}
#endif
