#ifndef _TREE_H
#define _TREE_H
#include "ds_common.h"

#define max(a, b) (a) > (b) ? a : b

typedef enum {
	DEF_SORT,
	SORT_ASC,
	SORT_DESC
} sort_t;


typedef struct tree_node {
	void *data;
	struct tree_node *lft; 
	struct tree_node *rgt;
	int height;
} tree_node_t;

typedef struct tree {
	tree_node_t *root;
	cmp_func_t cmp_func;
	del_func_t del_func;
} tree_t;



tree_t *tree_open(cmp_func_t cmp_func, del_func_t del_func);

int tree_close(tree_t *tree);

int tree_add(tree_t *tree, void *data);

int tree_del(tree_t *tree, void *data);

int tree_find(tree_t *tree, void *data);

void tree_foreach(tree_t *tree, visit_func_t visit_func, sort_t sort, void *ctx);

int tree_node_bf(tree_node_t *node);

#endif
