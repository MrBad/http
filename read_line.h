#ifndef _READ_LINE
#define _READ_LINE


#define CBSIZE 1526

// a circular buffer //
typedef struct cbuf {
	char buf[CBSIZE];
	int fd; 
	ssize_t rpos, wpos;
} cbuf_t;

// reads all size bytes into dest //
int readn(cbuf_t *cbuf, char *dst, unsigned int size);

// writes all size bytes from src //
int writen(cbuf_t *cbuf, char *src, unsigned int size);

// reads size bytes into dest
// stops when size-1 reached or \n
// and null terminate dst buffer
// keeps the rest into buffer so next read_line/readn will not be 
// affected
int read_line(cbuf_t *cbuf, char *dst, unsigned int size);

#endif
