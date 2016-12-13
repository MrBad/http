
typedef struct node_t {
	struct node_t *prev, *next;
	void *data;
} node_t;


// common function to free objects data
typedef void (*free_function_t) (void *data); 
// common function to be called on each node in for_each
//typedef int (*iterator_function_t) (node_t *node, va_list *app);
typedef int (*iterator_function_t) (node_t *node);
typedef int (*compare_function_t) (node_t *node, void *what);

typedef struct {
	node_t *head;
	node_t *tail;
	unsigned int num_items;
	free_function_t free_function;
} list_t;





list_t *list_open(free_function_t free_function);
void list_close(list_t *list);
node_t *list_add(list_t *list, void *data);
void list_del(list_t *list, node_t *node);
node_t *list_for_each(list_t *list, iterator_function_t iterator);

node_t *list_find(list_t *list, compare_function_t compare, void * what);
