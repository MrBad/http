#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"

static tree_node_t *tree_node_new(void *data) 
{
	tree_node_t *node;
	if(!(node = malloc(sizeof(*node)))) {
		perror("malloc");
		return NULL;
	}
	node->data = data;
	node->lft = node->rgt = NULL;
	node->height = 0;
	return node;
}

static int 
tree_node_height(tree_node_t *node) 
{
	int lh, rh;
	if(!node) return 0;
	lh = node->lft ? 1 + node->lft->height : 0;
	rh = node->rgt ? 1 + node->rgt->height : 0;
	return rh > lh ? rh : lh;	
}


// returns balance factor //
int tree_node_bf(tree_node_t *node)
{
	int lh, rh;
	if(!node) return 0;
	lh = node->lft ? 1 + node->lft->height : 0;
	rh = node->rgt ? 1 + node->rgt->height : 0;
	return lh - rh;
}

tree_node_t *rotate_left(tree_node_t *node) 
{
	tree_node_t *rnode;
	rnode = node->rgt;
	node->rgt = rnode->lft;
	rnode->lft = node;
	node->height = tree_node_height(node);
	rnode->height = tree_node_height(rnode);
	return rnode;
}

tree_node_t *rotate_right(tree_node_t *node) {
	tree_node_t *lnode;
	lnode = node->lft;
	node->lft = lnode->rgt;
	lnode->rgt = node;
	node->height = tree_node_height(node);
	lnode->height = tree_node_height(lnode);
	return lnode;
}

static tree_node_t *
tree_node_add(tree_node_t *root, void *data, cmp_func_t cmp_func) 
{
	if(!(root)) {
		root = tree_node_new(data);
	}
	else if(cmp_func(data, root->data) <= 0) {
		root->lft = tree_node_add(root->lft, data, cmp_func);
		if(tree_node_bf(root) >= 2) {
			if(cmp_func(data, root->lft->data) <= 0) { // is left left heavy //
				root = rotate_right(root);
			} else { // is left-right heavy //
				root->lft = rotate_left(root->lft);
				root = rotate_right(root);
			}
		}
	}
	else {
		root->rgt = tree_node_add(root->rgt, data, cmp_func);	
		if(tree_node_bf(root) <= -2) {
			if(cmp_func(data, root->rgt->data) > 0) { // is right right heavy //
				root = rotate_left(root);
			} else { // is right left heavy //
				root->rgt = rotate_right(root->rgt);
				root = rotate_left(root);
			}
		}
	}
	
	root->height = tree_node_height(root);

	return root;
}

static tree_node_t *tree_node_min(tree_node_t *root) 
{
	tree_node_t *node = NULL;
	if(root)
		for(node = root; node->lft; node = node->lft);
	return node;
}
#if 0
int itms;
static tree_node_t *
tree_node_del(tree_node_t *root, void *data, cmp_func_t cmp_func)
{
	int res;
	tree_node_t *node;
	if(!root) return NULL;
	
	res = cmp_func(data, root->data);
	
	if(res < 0) { // delete from left 
		printf("l");
		root->lft = tree_node_del(root->lft, data, cmp_func);
/*		if(tree_node_bf(root) <= -2) {
			if(tree_node_bf(root->rgt) <= 0) { // right right
				printf(";-rrh-;");
				root = rotate_left(root);
			} else { // right left
				printf(";-rlh-;");
				root->rgt = rotate_right(root->rgt);
				root = rotate_left(root);
			}
		}
*/	}	
	else if(res > 0) { // delete from right
		printf("r");
		root->rgt = tree_node_del(root->rgt, data, cmp_func);
/*		if(tree_node_bf(root) >= 2) {
			if(tree_node_bf(root->lft) >= 0) { // left left heavy
				printf(";+llh+;");
				root = rotate_right(root);
			} else { // left right
				printf(";-lrh-;");
				root->lft = rotate_left(root->lft);
				root = rotate_right(root);
			}
		}
*/	} 
	else {
		printf("f");

		itms++;
		if(!root->lft && !root->rgt) {
			node = root->lft ? root->lft : root->rgt;
			free(root);
			return node;
		}
		node = tree_node_min(root->rgt);
		printf("node->height: %d\n", node->height);
		root->data = node->data;
		root->rgt = tree_node_del(root->rgt, node->data, cmp_func);
		/*	if(tree_node_bf(root) >= 2) {
			if(tree_node_bf(root->lft) >= 0) { // left left heavy
			printf(";ll heavy;");
			root = rotate_right(root);
			} else { // left right
			printf(";lr heavy;");
			root->lft = rotate_left(root->lft);
			root = rotate_right(root);
			}
			}
			*/
	}
	root->height = tree_node_height(root);
	return root;
}
#endif

static tree_node_t *
tree_node_del(tree_node_t *root, void *data, cmp_func_t cmp_func) 
{
	tree_node_t *n;
	if(root) {
		int cmp = cmp_func(data, root->data);
		if(cmp < 0) { // go to left
			root->lft = tree_node_del(root->lft, data, cmp_func);
			if(tree_node_bf(root) <= -2) {
				if(tree_node_bf(root->rgt) <= 0) { // right right
					root = rotate_left(root);
				} else { // right left
					root->rgt = rotate_right(root->rgt);
					root = rotate_left(root);
				}
			}
		} else if(cmp > 0) {
			root->rgt = tree_node_del(root->rgt, data, cmp_func);
			if(tree_node_bf(root) >= 2) {
				if(tree_node_bf(root->lft) >= 0) { // left left heavy
					root = rotate_right(root);
				} else { // left right
					root->lft = rotate_left(root->lft);
					root = rotate_right(root);
				}
			}
		} else {
			if(!root->lft) {
				n = root->rgt;
				free(root);
				return n;
			} else if(!root->rgt) {
				n = root->lft;
				free(root);
				return n;
			}
			n = tree_node_min(root->rgt);
			root->data = n->data;
			root->rgt = tree_node_del(root->rgt, root->data, cmp_func);
			if(tree_node_bf(root) >= 2) {
				if(tree_node_bf(root->lft) >= 0) { // left left heavy
					root = rotate_right(root);
				} else { // left right
					root->lft = rotate_left(root->lft);
					root = rotate_right(root);
				}
			}
		}
	}
	root->height = tree_node_height(root);
	return root;
}

static tree_node_t *
tree_node_find(tree_node_t *root, void *data, cmp_func_t cmp_func) 
{
	tree_node_t *node = NULL;
	int res = 0; 
	node = root;
	if(root)
		res = cmp_func(data, root->data);
	if(res < 0) 
		node = tree_node_find(root->lft, data, cmp_func);
	else if(res > 0)
		node = tree_node_find(root->rgt, data, cmp_func);

	return node;
}

void tree_node_foreach(tree_node_t *node, visit_func_t vfunc, 
		sort_t sort, void *ctx) 
{
	if(!node) return;
	if(sort == SORT_ASC) {
		if(node->lft)
			tree_node_foreach(node->lft, vfunc, sort, ctx);
		vfunc(node->data, ctx);
		if(node->rgt)
			tree_node_foreach(node->rgt, vfunc, sort, ctx);
	} else if(sort == SORT_DESC) {
		if(node->rgt)
			tree_node_foreach(node->rgt, vfunc, sort, ctx);
		vfunc(node->data, ctx);
		if(node->lft)
			tree_node_foreach(node->lft, vfunc, sort, ctx);
	} else {
		vfunc(node->data, ctx);
		if(node->lft)
			tree_node_foreach(node->lft, vfunc, sort, ctx);
		if(node->rgt)
			tree_node_foreach(node->rgt, vfunc, sort, ctx);
	}
}


tree_t *tree_open(cmp_func_t cmp_func, del_func_t del_func) 
{
	tree_t *tree;
	if(!(tree = malloc(sizeof(*tree)))) {
		perror("malloc");
		return NULL;
	}
	tree->root = NULL;
	tree->cmp_func = cmp_func;
	tree->del_func = del_func;
	return tree;
}

int tree_close(tree_t *tree) 
{
	while(tree->root) {
		tree->root = tree_node_del(tree->root, tree->root->data, tree->cmp_func);
	};
	free(tree);
	return 0;
}

int tree_add(tree_t *tree, void *data) 
{
	tree->root = tree_node_add(tree->root, data, tree->cmp_func);
	return tree->root ? 0 : -1;
}

int tree_del(tree_t *tree, void *data) 
{
	tree->root = tree_node_del(tree->root, data, tree->cmp_func);
	return tree->root ? 0 : -1;	
}

int tree_find(tree_t *tree, void *data) 
{
	tree_node_t *node = tree_node_find(tree->root, data, tree->cmp_func);	
	return node ? 0 : -1;
}

void tree_foreach(tree_t *tree, visit_func_t visit_func, sort_t sort, void *ctx)
{
	tree_node_foreach(tree->root, visit_func, sort, ctx);
}


#ifdef TEST_TREE
int string_compare(void *s1, void *s2) 
{
	return strcmp((char*)s1, (char*)s2);
}

int print_node(void *data, void *context) 
{
	(void)context;
	printf("%s\n", (char *)data);
	return 0;
}

int main()
{
	char *str[] = {
		"popescu",
		"ionescu",
		"other",
		"vasilescu",
		"blah blah",
		"testing",
		"zamolxe",
		"mrbadnews"
	};
	unsigned int i;
	tree_t *names = tree_open(&string_compare, NULL);
	for(i = 0; i < sizeof(str)/sizeof(*str); i++) {	
		tree_add(names, str[i]);	
	}
	char *search = "testing";
	printf("%s exists: %s\n", search, tree_find(names, search) < 0 ? "no":"yes");
	printf("%s exists: %s\n", "zzz", tree_find(names, "zzz") < 0 ? "no":"yes");
	printf("_____  asc traverse _____\n");	
	tree_foreach(names, &print_node, SORT_ASC, NULL);
	printf("_____ desc traverse _____\n");	
	tree_foreach(names, &print_node, SORT_DESC, NULL);
	printf("_____ default traverse ____\n");
	tree_foreach(names, &print_node, DEF_SORT, NULL);
	tree_close(names);
	
	return 0;
}
#endif
