#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"


typedef struct employee {
	char *fname;
	char *lname;
	int age; // int because i am lazy
} emp_t;


emp_t *new_emp(char *first_name, char *last_name, int age) {
	emp_t *e;
	if(!(e = malloc(sizeof(*e)))) {
		perror("malloc");
		exit(1);
	}
	e->fname = strdup(first_name);
	e->lname = strdup(last_name);
	e->age = age;
	return e;
}

void del_item(void *data) {
	emp_t *e = data;
	free(e->fname);
	free(e->lname);
}

int print_emp(void *data, void *ctx) {
	(void) ctx;
	emp_t *e = data;
	printf("%s %s %d\n", e->fname, e->lname, e->age);
	return 0;
}

#define len(a) sizeof(a) / sizeof(*a)

int emp_cmp_fname(void *data, void *fname) {
	emp_t *e = data;
	if(strcmp(e->fname, (char *)fname))
		return 0;
	return -1;	
}

int main()
{
	emp_t employees[] = {
		{"popescu", "ion", 22},
		{"vasilescu", "gheorghe", 33},
		{"john", "doe", 42}
	};
	unsigned int i;
	list_t *list = list_open(del_item);
	for (i = 0; i < len(employees); i++) {
		list_add(list, new_emp(employees[i].fname, employees[i].lname, 
					employees[i].age));
	}
	
	list_node_t *n;
	for(n = list->head; n; n = n->next) {
		emp_t *e = n->data;
		printf("%s %s %d\n", e->fname, e->lname, e->age);
	}
	// or //
	list_foreach(list, n) {
		printf("%s\n", (char *)((emp_t*) n->data)->fname);
	}

	list_close(list);
}
