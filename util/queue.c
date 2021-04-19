#include "queue.h"

bool isEmpty(queue *q) {
    if (q->front == NULL)
        return true;
    return false;
}

q_node *front(queue *q) {
    if (isEmpty(q))
        return NULL;

    return q->front;
}

bool isFull(queue *q) {
    if (q->node_count == q->max_count)
        return true;
    return false;
}

q_node *newNode(void *data) {
    q_node *temp = (q_node *) malloc(sizeof(q_node));
    temp->data_ptr = data;
    temp->next = NULL;
    return temp;
}

queue *createQueue(int max_count) {
    queue *q = (queue *) malloc(sizeof(queue));
    q->front = q->rear = NULL;
    q->max_count = max_count;
    q->node_count = 0;
    return q;
}

void push(queue *q, void *data) {
    // If maximum limit on number of nodes is reached
    if (isFull(q))
        pop(q);

    q_node *temp = newNode(data);
    q->node_count++;

    // If queue is empty, then new node is front and rear both
    if (isEmpty(q)) {
        q->front = q->rear = temp;
        return;
    }

    //  Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
}

q_node *pop(queue *q) {
    if (isEmpty(q))
        return NULL;

    // Store previous front and move front one node ahead
    q_node *temp = q->front;
    q->front = q->front->next;
    q->node_count--;

    // If front becomes NULL, then change the rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;

    return temp;
}