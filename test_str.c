#include <stdio.h>
#include <string.h>
#include "str.h"


int main(){
	str_t *s = strNew("Testing");
	str_t *s2 = strNew(" this thing \n");
	str_t *s3 = strNew(NULL);

	strCat(s, s2);
	strCpy(s3, s);
	strAppend(s3, "aaaaa");
	printf("%lu, %lu, %lu %s\n", s->len, s->size, strlen(s->str), s->str);
	printf("%lu, %lu, %lu %s\n", s3->len, s3->size, strlen(s3->str), s3->str);

	str_t *c = strNew("testing");

	strCpy(s, c);
//	strCpy(s2, c);
	printf("compare: %d\n", strCmp(s, s2));

	strDel(s);
	strDel(s2);
	strDel(s3);
	return 0;
}
