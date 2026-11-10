#ifndef QUEUE_H
#define QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct QueueNode {
    void *data;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *front;
    QueueNode *rear;
    int count;
} Queue;

typedef void (*QueueVisitor)(void *data, void *context);

void queue_init(Queue *q);
int queue_enqueue(Queue *q, void *data);
void *queue_dequeue(Queue *q);
void *queue_peek(const Queue *q);
int queue_size(const Queue *q);
void queue_for_each(const Queue *q, QueueVisitor visit, void *context);
void queue_destroy(Queue *q);

#ifdef __cplusplus
}
#endif

#endif
