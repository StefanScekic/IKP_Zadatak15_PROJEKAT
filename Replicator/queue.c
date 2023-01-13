#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

#define INITIAL_SIZE 10

Queue* createQueue() {
    Queue* q = malloc(sizeof(Queue));
    q->size = INITIAL_SIZE;
    q->front = q->rear = -1;
    q->array = malloc(q->size * sizeof(int));
    return q;
}

void resizeQueue(Queue* q) {
    int newSize = q->size * 2;
    int* newArray = malloc(newSize * sizeof(int));
    int i, j;

    // Preslikavanje elemenata iz starog niza u novi
    for (i = q->front, j = 0; i <= q->rear; i++, j++) {
        newArray[j] = q->array[i % q->size];
    }

    free(q->array);
    q->array = newArray;
    q->front = 0;
    q->rear = j - 1;
    q->size = newSize;
}

int isEmpty(Queue* q) {
    return (q->front == -1);
}

int isFull(Queue* q) {
    return ((q->rear + 1) % q->size == q->front);
}

void enqueue(Queue* q, int x) {
    if (isFull(q)) {
        resizeQueue(q);
    }
    q->rear = (q->rear + 1) % q->size;
    q->array[q->rear] = x;
    if (q->front == -1) {
        q->front = q->rear;
    }
}

int dequeue(Queue* q) {
    int x;
    if (isEmpty(q)) {
        printf("Red je prazan.\n");
        return -1;
    }
    x = q->array[q->front];
    if (q->front == q->rear) {
        q->front = q->rear = -1;
    }
    else {
        q->front = (q->front + 1) % q->size;
    }
    return x;
}