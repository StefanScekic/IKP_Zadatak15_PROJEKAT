#ifndef QUEUE_H
#define QUEUE_H

typedef struct {
    int* array;
    int size;
    int front;
    int rear;
} Queue;

Queue* createQueue();
void resizeQueue(Queue* q);
int isEmpty(Queue* q);
int isFull(Queue* q);
void enqueue(Queue* q, int x);
int dequeue(Queue* q);

#endif // QUEUE_H