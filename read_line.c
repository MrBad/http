#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include "read_line.h"
#include <sys/socket.h>

#if 0
#define CBSIZE 1526

typedef struct cbuf {
	char buf[CBSIZE];
	int fd;
	unsigned int rpos, wpos;
} cbuf_t;
#endif

int readn(cbuf_t *cbuf, char *dst, unsigned int size)
{
	ssize_t n;
	unsigned int i = 0;

	while (i <= size) {
		if (cbuf->rpos == cbuf->wpos) {
			// fill buf //
			size_t wpos = cbuf->wpos % CBSIZE;
			//if ((n = read(cbuf->fd, cbuf->buf + wpos, (CBSIZE - wpos))) < 0) {
			if ((n = recv(cbuf->fd, cbuf->buf + wpos, (CBSIZE - wpos), 0)) < 0) {
				if (errno == EINTR)
					continue;
				return -1;
			} else if (n == 0)
				return 0;
			cbuf->wpos += n;
		}
		dst[i++] = cbuf->buf[cbuf->rpos++ % CBSIZE];
	}
	return i-1;
}

int writen(cbuf_t *cbuf, char *src, unsigned int size)
{
	ssize_t  n;
	unsigned int i = 0;

	while(i < size) {
		if((n = write(cbuf->fd, src + i, size - i)) < 0) {
			if(errno == EINTR)
				continue;
			return -1;
		} else if(n == 0)
			return 0;
		i += n;
	}
	return i;
}

int read_line(cbuf_t *cbuf, char *dst, unsigned int size)
{
	unsigned int i = 0;
	ssize_t n;
	while(i < size) {
		if(cbuf->rpos == cbuf->wpos) {
			size_t wpos = cbuf->wpos % CBSIZE;
			if((n = read(cbuf->fd, cbuf->buf + wpos, (CBSIZE - wpos))) < 0) {
				if(errno == EINTR)
					continue;
				return -1;
			} else if(n == 0)
				return 0;
			cbuf->wpos += n;
		}
		dst[i++] = cbuf->buf[cbuf->rpos++ % CBSIZE];
		if(dst[i-1]=='\n')
			break;
	}
	dst[i] = 0;
	return i;
}

#ifdef RTEST
int main()
{
	int fd;
	char buf[512];
	if ((fd = open("/home/develop/cprogs/queue.db.1", O_RDONLY)) < 0) {
		perror("open");
		exit(1);
	}
	cbuf_t cbuf;
	cbuf.fd = fd;
	cbuf.wpos = cbuf.rpos = 0;
//	while (readn(&cbuf, buf, 9) == 9) {
//		buf[10] = 0;
//		printf("%s\n", buf);
//	}
	int n;
	while((n = read_line(&cbuf, buf, sizeof(buf)-1)) > 0) {
		printf("%s", buf);
	}
	close(fd);
}
#endif
