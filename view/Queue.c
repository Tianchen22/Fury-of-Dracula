// Queue ADT

#include <assert.h>
#include <memory.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "Queue.h"

static inline int get_q_index(int start, int total, int index) 
{
    index += start;
    index -= (index >= total) ? total : 0;
    return index;
}

static inline int get_q_index_backwards(int start, int total,
                                          int size, int index) 
{
    int index_l = get_q_index(start, total, size - 1);
    // avoid negative number because all are unsigned
    return (index_l < index) ? total + index_l - index: index_l - index;
}

queue_t *new_queue(int total) 
{    
    queue_t *q = malloc(sizeof(queue_t));
    q->total = total;
    q->val = malloc(total * sizeof(q_item_t));
    memset(q->val, 0, total);
    q->size = q->start = 0;
    return q;
}

queue_t *clone_queue(queue_t *q) 
{    
    queue_t *q_now = malloc(sizeof(queue_t));
    q_now->total = q->total;
    q_now->size = q->size;
    q_now->start = q->start;
    q_now->val = malloc(q_now->total * sizeof(q_item_t));
    memcpy(q_now->val, q->val, q_now->total * sizeof(q_item_t));

    return q_now;
}

void destroy_queue(queue_t *q) 
{    
    free(q->val);
    free(q);
}

q_item_t queue_get(queue_t *q, int index) 
{
    if (index >= q->size) return UNKNOWN_ITEM;
    return q->val[get_q_index(q->start, q->total, index)];
}

q_item_t queue_last(queue_t *q) 
{
    return queue_get_backwards(q, 0);
}

q_item_t queue_get_backwards(queue_t *q, int index) 
{
    if (index >= q->size) return UNKNOWN_ITEM;
    return q->val[get_q_index_backwards(q->start, q->total, q->size, index)];
}

void queue_add(queue_t *q, q_item_t item) 
{
    if (q->size < q->total)
        q->val[get_q_index(q->start, q->total, q->size++)] = item;
    else
        q->val[q->start++] = item;
    if (q->start == q->total) q->start = 0;
}

void queue_remove_first(queue_t *q)
{
    if (q->size == 0) return;
    q->start++, q->size--;
    if (q->start == q->total) q->start = 0;
}

int queue_size(queue_t *q) 
{
    return q->size;
}

int queue_to_array(queue_t *q, q_item_t arr[],bool reversed) 
{
    if (reversed) {
        for (int i = (int)q->start, j = (int)q->size - 1; j >= 0;
            i = (i + 1 == q->total ? 0 : i + 1), j--)
          arr[j] = q->val[i];
    } else {
        for (int i = (int)q->start, j = 0; j < q->size;
            i = (i + 1 == (int)q->total ? 0 : i + 1), j++)
          arr[j] = q->val[i];
    }
    for (int i = q->size; i < q->total; i++) arr[i] = UNKNOWN_ITEM;

    return q->size;
}

bool in_queue(queue_t *q, q_item_t item) 
{
    int m = (q->size < q->total) ? q->size : q->total;
    for (int i = 0; i < m; i++) {
        if (q->val[i] == item) return true;
    }
  
    return false;
}
