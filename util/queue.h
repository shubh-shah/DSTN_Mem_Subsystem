#ifndef ADT_QUEUE_H
#define ADT_QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


typedef struct node{
    void *data_ptr;
    struct node *next;
} q_node;

typedef struct {
    q_node *front;
    q_node *rear;
    int max_count;
    int node_count;
} queue;

extern bool isEmpty(queue *q);

extern q_node *front(queue *q);

extern void *rear(queue *q);

extern bool isFull(queue *q);

extern q_node *newNode(void *data);

extern queue *createQueue(int max_count);

extern void push(queue *q, void *data);

extern q_node *pop(queue *q);


#endif
