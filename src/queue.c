#include <stdlib.h>
#include "queue.h"

void queue_init(Queue *q) {
    q->front = NULL;
    q->rear = NULL;
    q->count = 0;
}

int queue_enqueue(Queue *q, void *data) {
    QueueNode *node;
    if (!data) {
        return -1;
    }
    node = malloc(sizeof *node);
    if (!node) {
        return -1;
    }
    node->data = data;
    node->next = NULL;
    if (q->rear) {
        q->rear->next = node;
    } else {
        q->front = node;
    }
    q->rear = node;
    q->count++;
    return 0;
}

void *queue_dequeue(Queue *q) {
    QueueNode *node = q->front;
    void *data;
    if (!node) {
        return NULL;
    }
    data = node->data;
    q->front = node->next;
    if (!q->front) {
        q->rear = NULL;
    }
    q->count--;
    free(node);
    return data;
}

void *queue_peek(const Queue *q) {
    return q->front ? q->front->data : NULL;
}

int queue_size(const Queue *q) {
    return q->count;
}

void queue_for_each(const Queue *q, QueueVisitor visit, void *context) {
    const QueueNode *n;
    for (n = q->front; n; n = n->next) {
        visit(n->data, context);
    }
}

void queue_destroy(Queue *q) {
    QueueNode *n = q->front;
    while (n) {
        QueueNode *next = n->next;
        free(n);
        n = next;
    }
    q->front = NULL;
    q->rear = NULL;
    q->count = 0;
}
