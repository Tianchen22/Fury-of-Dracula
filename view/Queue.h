// Queue ADT
#include <stddef.h>

#ifndef QUEUE_H_
#define QUEUE_H_

typedef int q_item_t;

#define UNKNOWN_ITEM (-2)

typedef struct queue {
    int total;
    q_item_t *val;
    int start;
    int size;
} queue_t;

queue_t *new_queue(int total);
queue_t *clone_queue(queue_t *q);
void destroy_queue(queue_t *q);
q_item_t queue_get(queue_t *q, int index);
q_item_t queue_last(queue_t *q);
q_item_t queue_get_backwards(queue_t *q, int index);
void queue_add(queue_t *q, q_item_t item);
void queue_remove_first(queue_t *q);
int queue_size(queue_t *q);
int queue_to_array(queue_t *q, q_item_t arr[],bool reversed);
bool in_queue(queue_t *q, q_item_t item);

#endif  // !defined (QUEUE_H_)