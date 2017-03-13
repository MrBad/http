#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include <mysql/mysql.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#if 0
#define SQL_HOST "localhost"
#define SQL_USER "root"
#define SQL_PASS "fantasmadelux"
#define SQL_DB	"newsrt"

int fetch_data(tree_t *tree)
{
	MYSQL *sql;
	MYSQL_RES *res;
	MYSQL_ROW row;
	if(!(sql = mysql_init(NULL))) {
		perror("mysql_init");
		return -1;
	}
	if(!mysql_real_connect(sql, SQL_HOST, SQL_USER, SQL_PASS, SQL_DB, 0,NULL,0)) {
		mysql_close(sql);
		perror("mysql_real_connect");
		return -1;
	}
	if(mysql_query(sql, "SELECT url FROM queue LIMIT 500000")) {
		perror("mysql_query");
		return -1;
	}
	if(!(res = mysql_store_result(sql))) {
		perror("mysql_store_result");
		return -1;
	}
	while((row = mysql_fetch_row(res))) {
		if(row[0])
			tree_add(tree, strdup(row[0]));
	}

	mysql_free_result(res);
	mysql_close(sql);
	return 0;
}
#endif

unsigned int min_len = -1;

void del_str(void *str) {
	free(str);
}

int cmp_str(void *s1, void *s2) {
	return strcmp((char *) s1+min_len, (char *) s2+min_len);
}

int save_url(void *url, void *ctx) {
	int *fd =  ctx;
	int n = 0;
	if(!url) return -1;
	n = write(*fd, url, strlen((char*)url));
	n+=write(*fd, "\n", 1);
	return n;
}

int save_urls(tree_t *tree) {
	void *ctx;
	int fd = open("tree.db", O_WRONLY|O_TRUNC|O_CREAT, 0666);
	if(!fd) {
		perror("open");
		return -1;
	}
	ctx = &fd;
	tree_foreach(tree, save_url, 0, ctx);
	close(fd);
	return 0;
}

#define min(a, b) a < b ? a : b
int load_urls(char *file, tree_t *tree) 
{
	FILE *fp;
	char buf[4096];
	int i = 0;
	printf("Loading data from file %s ", file);
	fflush(stdout);
	if(!(fp = fopen(file, "r"))) {
		perror("fopen");
		return -1;
	}
	while(fgets(buf, sizeof(buf), fp)) {
		if(i % 10000 == 0) write(0, ".", 1);
		unsigned int len = strlen(buf);
		buf[len-1] = 0;
		min_len = min(min_len, len);
		tree_add(tree, strdup(buf));
		i++;
	}
	printf(" OK\n");
	fclose(fp);
	return 0;
}

int main()
{
	tree_t *urls = tree_open(cmp_str, del_str);
	/*if(fetch_data(urls) < 0) {
		fprintf(stderr, "cannot fetch data\n");
		exit(1);
	}*/
	int min_len;
	min_len	= load_urls("tree.db", urls);
	printf("min len is: %d\n", min_len);
	char *str = "http://metro.co.uk/2016/08/24/rapists-dad-threatened-to-kill-witness-with-rounders-bat-6087084/";
	printf("%s exists? %s\n", str, tree_find(urls, str)==0 ? "yes":"no");
	if(tree_find(urls, str) == 0) {
		tree_del(urls, str);
	}
		
	printf("%s exists? %s\n", str, tree_find(urls, str)==0 ? "yes":"no");
	printf("tree height is: %d\n", urls->root->height);
	printf("root balance factor: %i", tree_node_bf(urls->root->rgt));
	//save_urls(urls);
	
	tree_close(urls);

	return 0;
}
