#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "buf.h"


int main()
{
	buf_t *buf;
	buf = buf_new(0);
	char *s = "buffers, aheeey";
	buf_sprintf(buf, "%s", s);

	printf("len: %lu, size: %lu, strlen: %lu, [%s]\n",
			buf->len, buf->size, strlen(buf->data), buf->data);
	
	char *abcd = "abcd";
	buf_sprintf(buf, "%s", abcd);
	printf("len: %lu, size: %lu, strlen: %lu, [%s]\n",
			buf->len, buf->size, strlen(buf->data), buf->data);

	return 0;
	int a = 20;
	buf_append(buf, "a little project. ", 18);
	buf_sprintf(buf, "testing %s this %d\n", s, a);
	printf("len: %lu, size: %lu\n", buf->len, buf->size);
	buf_expand(buf, 4096);
	printf("len: %lu, size: %lu, s: %s\n", buf->len, buf->size, buf->data);
	printf("%s\n", buf->data);
}
