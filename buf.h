#ifndef _BUF_H
#define _BUF_H

typedef struct {
	char *data;
	size_t len;
	size_t size;
} buf_t;


buf_t *buf_new(size_t size);
int buf_del(buf_t *buf);
int buf_expand(buf_t *buf, size_t size);
int buf_append(buf_t *buf, char *src, size_t n);
int buf_sprintf(buf_t *buf, char *format, ...);
void buf_reset(buf_t *buf);
#endif
