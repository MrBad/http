#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include "parse_url.h"
#include "queue.h"

#define SQL_HOST "localhost"
#define SQL_USER "root"
#define SQL_PASS "mypass"
#define SQL_DB "test"

typedef struct url_t {
	char url[256];
	char domain[64];
	unsigned int added_ts;
} url_t;

url_t *new_url(char *url) {
	url_t *n;
	struct url_parts_t *parts;
	n = (url_t *) malloc(sizeof(url_t));

	if(!n) {
		perror("malloc");
		return NULL;
	}
	parts = parse_url(url);
	strncpy(n->url, url, sizeof(n->url)-1);
	strncpy(n->domain, parts->host, sizeof(n->domain)-1);
	// free parts //
	return n;
}

int main(void) {
	MYSQL *sql;
	queue_t *queue;
		
	queue = queue_open();
	if(!queue) {
		perror("queue_open");
		return -1;
	}
	
	sql = mysql_init(NULL);	
	if(!sql) {
		fprintf(stderr, "mysql_init, %s\n", mysql_error(sql));
		return 1;
	}	
	
	if(mysql_real_connect(sql, SQL_HOST, SQL_USER, SQL_PASS, SQL_DB, 0, NULL, 0) == NULL) {
		fprintf(stderr, "%s\n", mysql_error(sql));
		mysql_close(sql);
		return 1;
	}

	if(mysql_query(sql, "SELECT url, domain FROM queue LIMIT 100")) {
		fprintf(stderr, "%s\n", mysql_error(sql));
		return 1;
	}

	MYSQL_RES *result = mysql_store_result(sql);
	if(! result) {
		fprintf(stderr, "%s\n", mysql_error(sql));
		return 1;
	}
	//int num_fields = mysql_num_fields(result);
	MYSQL_ROW row;
	while((row = mysql_fetch_row(result))) {
		url_t *url = new_url(row[0]);
		queue_item_t *node = new_node(url);
		queue_add(queue, node);
	}

	mysql_close(sql);

	queue_item_t *n;
	n = queue->head;
	while(n) {
		url_t *u;
		u = (url_t *) n->data;
		printf("%s, %s\n", u->domain, u->url);
		n = n->next;
	}

	queue_close(queue);

	return 0;
}
