#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "buf.h"

buf_t *buf_new(size_t size) 
{
	buf_t *buf = NULL;
	if(!(buf = malloc(sizeof(*buf)))) {
		perror("malloc");
		return buf;
	}
	buf->len = 0;
	if(!size)
		size = 1024;
	if(!(buf->data = malloc(size))) {
		perror("malloc");
		free(buf);
		return NULL;
	}
	buf->size = size;
	return buf;
}

int buf_del(buf_t *buf) 
{
	if(!buf->data)
		return -1;
	free(buf->data);
	free(buf);
	return 0;
}

int buf_expand(buf_t *buf, size_t size) 
{
	char *d;
	if(size == 0) {
		size = buf->size * 2;
	}
	if(size <= buf->size) {
		fprintf(stderr, "buf_expand: size <= buf->size\n");
		return -1;
	}
	if(!(d = realloc(buf->data, size))) {
		perror("realloc");
		return -1;
	}
	buf->data = d;
	buf->size = size;
	return 0;
}

int buf_append(buf_t *buf, char *src, size_t n)
{
	if(buf->size < buf->len + n) {
		if(buf_expand(buf, buf->len + n) < 0) {
			fprintf(stderr, "buf_append: error expanding buffer\n");
			return -1;
		}
	}
	memcpy(buf->data+buf->len, src, n);	
	buf->len += n;
	return 0;
}

int buf_sprintf(buf_t *buf, char *format, ...) 
{
	va_list args;
	int len;
	// compute needed len //
	va_start(args, format);
	len = vsnprintf(NULL, 0, format, args);
	if(buf->size < buf->len + len + 1) {
		if(buf_expand(buf, buf->len + len + 1) < 0) {
			fprintf(stderr, "buf_sprintf: error expanding buffer\n");
			return -1;
		}
	}
	va_end(args);
	va_start(args, format);
	if(vsnprintf((buf->data + buf->len), len+1, format, args) != len) {
		fprintf(stderr, "buf_sprintf: error in snprintf\n");
		return -1;
	}
	va_end(args);
	buf->len += len;
	buf->data[buf->len] = 0; // expect string //
	return len;
}
void buf_reset(buf_t *buf) 
{
	buf->len = 0;
}
