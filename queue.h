typedef struct queue_item_t {
	void *data;
	struct queue_item_t *prev;
	struct queue_item_t *next;
} queue_item_t;

typedef struct {
	queue_item_t *head;
	queue_item_t *tail;
	int num_items;
} queue_t;


queue_t *queue_open();

int	queue_close(queue_t *);

void queue_add(queue_t *, queue_item_t *node);

void queue_del(queue_t *queue, queue_item_t *node);

queue_item_t *new_node(void *data);


