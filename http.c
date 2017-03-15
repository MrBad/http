#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>
#include <errno.h>
#include "http.h"
#include "parse_url.h"
#include "buf.h"

#define min(a, b) (a) < (b) ? a : b
#define max(a, b) (a) > (b) ? a : b

//
// HTTP //
// only supports GET 
// no keepalive, no cookies
// supports transfer chunked
//


char *getHost(char *host) 
{
	struct hostent *h = gethostbyname(host);
	if(!h) {
		return NULL;
	}
	return inet_ntoa(*(struct in_addr*)h->h_addr);
}

response_t *response_new() 
{
	response_t *res;
	if(!(res = calloc(1, sizeof(*res)))) {
		perror("malloc");
		return NULL;
	}
	res->length = -1;
	return res;
}

void response_del(response_t *res) 
{
	if(!res) return;
	if(res->header)
		free(res->header);
	if(res->body)
		free(res->body);
	if(res->type) free(res->type);
	if(res->subtype) free(res->subtype);
	if(res->redir_url) free(res->redir_url);
	if(res->last_url) free(res->last_url);
	free(res);
}

request_t *request_new()
{
	request_t *req;
	if(!(req = calloc(1, sizeof(*req)))) {
		perror("malloc");
		return NULL;
	}
	return req;
}

void request_del(request_t *req)
{
	free(req->url);
	if(req->header)
		free(req->header);	
	if(req->user_agent)
		free(req->user_agent);
}

// oh yes, i don't want to use regexps 
// because parsing strings is fun //
int parse_headers(response_t *res) 
{
	char line[512], *p, *k, *n, *hstr;
	char key[64];
	int len;
	if(!res->header) 
		return -1;
	hstr = res->header;

	while((p = strchr(hstr, '\n'))) {
		if((unsigned)(p-hstr) > sizeof(line)-1) {
			fprintf(stderr, "line too large");
			return -1;
		}
		line[0] = 0;
		len = p-hstr;
		strncpy(line, hstr, len);
		line[len] = 0;
		while(line[len-1] == '\r' || line[len-1]=='\n')
			line[--len] = 0;
		line[len] = 0;
		
		hstr = p+1;
		
		k = strchr(line, ':');
		if(!k) {
			// get status //
			if(strncmp("HTTP", line, 4) == 0) {
				k = line+4;
				while(*k && *k != ' ')k++;
				while(*k == ' ') k++;
				if(*k >= '0' && *k <= '9') {
					while(*k >='0' && *k <= '9') {
						res->status = 10 * res->status + *k-'0';
						k++;
					}
				}
			}
			continue;
		
		}
		if((unsigned)(k-line) > sizeof(key)-1) return -1;
		strncpy(key, line, k-line);
		key[k-line] = 0;
		k++;
		while(*k == ' '|| *k == '\t') k++;
		
		// get content type //
		if(strcasecmp("content-type", key) == 0) {
			n = k;
			while(*k && *k != '/' && *k != ';') k++;
			if(!(res->type = malloc(k-n+1))) {
				perror("malloc");
				return -1;
			}
			strncpy(res->type, n, k-n);
			res->type[k-n] = 0;
			if(*k == '/') {
				k++; n = k;
				while(*k && *k!=';') k++;
				if(!(res->subtype = malloc(k-n+1))) {
					perror("malloc"); return -1;
				}
				strncpy(res->subtype, n, k-n);
				res->subtype[k-n] = 0;
			}
			if(*k == ';') {
				k++;
				if(strncasecmp("charset", k, 7) == 0) {
					k+=7;
					if(*k=='=') k++;
					res->charset = strdup(k);
				}
			}
		} 

		// transfer encoding //
		else if(strcasecmp("transfer-encoding", key) == 0) {
			n = k;
			if(strncasecmp(n, "chunked", 7) == 0) {
				res->isChunked = 1;
			}
		} 

		// content length //
		else if(strcasecmp("content-length", key) == 0) {
			errno = 0;
			res->length = strtol(k, &n, 10);
			if(errno != 0 || k == n) {
				perror("strtol");
				return -1;
			}
		}

		// Location
		else if(strcasecmp("location", key) == 0) {
			n = k;
			while(!isspace(*k)) k++;
			if(!(res->redir_url = malloc(k-n+1))) {
				perror("malloc");
				return -1;
			}
			strncpy(res->redir_url, n, k-n);
			//printf("REDIR: {%s}\n", res->redir_url);
		}
	}

	// check minimum required headers //
	if(!res->status) {
		fprintf(stderr, "Cannot get status!\n");
		return -1;
	}
	if(res->status == 200) {
		if(!res->type) {
			fprintf(stderr, "Cannot get type!\n");
			return -1;
		}
		if(!res->subtype) {
			fprintf(stderr, "Cannot get subtype!\n");
			return -1;
		}
	}
	return 0;
}


int http_connect(char *ip, int port) 
{
	int fd;
	struct sockaddr_in saddr;
	fprintf(stdout, "Connecting to ip: %s\n", ip);
	if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return -1;
	}
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	inet_aton(ip, &saddr.sin_addr);
	if(connect(fd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
		perror("connect");
		return -1;
	}
	return fd;
}

int buf_write(buf_t *buf, FILE *fp, int num_bytes)
{
	int written = 0, left = num_bytes, n;
	do {
		if((n = fwrite(buf->data + written, 1, left, fp)) < 0) {
			perror("write");
			return -1;	
		}
		left -= n;
		written += n;
	} while(left > 0);

	return written;
}

int buf_read(buf_t *buf, FILE *fp, int num_bytes)
{
	int nread = 0, left = num_bytes, n;
	if(buf->size < buf->len + num_bytes + 1) {
		buf_expand(buf, buf->len + num_bytes + 1);
	}
	do {
		if((n = fread(buf->data + buf->len, 1, left, fp)) < 0) {
			perror("fread");
			return -1;
		}
		nread += n;
		left -= n;
		buf->len += n;
	} while(left > 0 || feof(fp));
	buf->data[buf->len] = 0;	
	return nread;
}

response_t *getLink(request_t *req) 
{
	response_t *res = NULL;
	url_parts_t *url_parts = NULL;
	buf_t *buf = NULL;
	char *ip = NULL;
	char line[512];
	int fd = -1, n;
	FILE *fp = NULL;

	if(!(url_parts = parse_url(req->url))) {
		fprintf(stderr, "getLink: cannot parse url: %s\n", req->url);
		goto fail;
	}
	if(!strcmp(url_parts->scheme, "http") == 0) {
		fprintf(stderr, "getLink: %s - sorry, only http support yet\n", req->url);
		goto fail;
	}
	if(!(res = response_new())) {
		fprintf(stderr, "getLink: response_new\n");
		goto fail;
	}
	if(!(buf = buf_new(128))) {
		fprintf(stderr, "getLink: buf_new\n");
		goto fail;
	}
	if(!(ip = getHost(url_parts->host))) {
		fprintf(stderr, "getLink: cannot resolve %s\n", url_parts->host);
		goto fail;
	}
	if((fd = http_connect(ip, url_parts->port)) < 0) {
		fprintf(stderr, "Cannot connect to %s\n", url_parts->host);
		goto fail;
	}
	if(!(fp = fdopen(fd, "r+"))) {
		perror("fdopen");
		goto fail;
	}
	fprintf(stdout, "Connected to %s\n", ip);

	// sending request //
	buf_sprintf(buf, "GET %s%s%s HTTP/1.1\r\nHost: %s\r\n", 
			url_parts->path,
			url_parts->query_string?"?":"",
			url_parts->query_string?url_parts->query_string:"",
			url_parts->host
			);
	buf_sprintf(buf, "User-Agent: %s\r\n", 
			req->user_agent ? req->user_agent : DEFAULT_USER_AGENT);
	buf_sprintf(buf, "Connection: close\r\n\r\n");
	// save a copy //
	req->header = strdup(buf->data);
	n = buf_write(buf, fp, buf->len);
	if((unsigned)n != buf->len) {
		fprintf(stderr, "Error writing buf\n");
		goto fail;
	}

	buf_reset(buf);

	// read header //
	do {
		if(!fgets(line, sizeof(line), fp)) {
			perror("in read header fgets");
			goto fail;
		}
		if(strcmp(line, "\r\n") == 0) {
			// header end //
			if(!(res->header = malloc(buf->len + 1))) {
				perror("in header, malloc");
				goto fail;
			}
			strncpy(res->header, buf->data, buf->len);
			res->header[buf->len] = 0;
			if(parse_headers(res) < 0) {
				fprintf(stderr, "cannot parse headers\n");
				goto fail;
			}
			break;
		}
		buf_append(buf, line, strlen(line));
	} while(1);

	buf_reset(buf);buf->data[0]=0;

	// read body //
	if(res->isChunked) {
		unsigned int chunk_size;
		do {
			if(!fgets(line, sizeof(line), fp)) {
				perror("in body fgets");
				goto fail;
			}
			if((sscanf(line, "%x\r\n", &chunk_size)) < 1) {
				fprintf(stderr, "Cannot read chunk_size\n");
				goto fail;
			}
			if(chunk_size == 0)
				break;
			n = buf_read(buf, fp, chunk_size);
			if((unsigned)n != chunk_size) {
				fprintf(stderr, "Cannot read chunk: got only %d from %d\n", n, chunk_size);
				goto fail;
			}
			if(fgetc(fp)!='\r' || fgetc(fp)!='\n') {
				fprintf(stderr, "chunk is not delimited\n");
				goto fail;
			}
		} while(!feof(fp));
		res->length = buf->len;
	}

	// we have the header length //
	else if(res->length > 0) {
		n = buf_read(buf, fp, res->length);
		if((unsigned)n != res->length) {
			fprintf(stderr, "Incomplete body, read only %d from %lu\n", n, res->length);
			goto fail;
		}
	} 
	else {
		// maybe i will loop until feof
		if(res->status == 200) {
			printf("No idea how to get body $ %s\n", res->header);
			goto fail;
		}
	}

	if(res->status == 200) {	
		if(buf->len == 0) {
			fprintf(stderr, "Error in getting body\n");
			goto fail;
		}
		if(!(res->body = malloc(buf->len + 1))) {
			perror("malloc");
			goto fail;
		}
		memcpy(res->body, buf->data, buf->len);
	}

	free_url_parts(url_parts);
	buf_del(buf);
	fclose(fp);	
	
	return res;


fail:
	if(url_parts)	
		free_url_parts(url_parts);
	if(res)
		response_del(res);
	if(buf)
		buf_del(buf);
	if(fp)
		fclose(fp);
	else if(fd > 0) 
		close(fd);
	return NULL;
}
