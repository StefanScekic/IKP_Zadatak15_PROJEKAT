#include <stdio.h>
#include<stdlib.h>
#include <stdlib.h>
#include <string>
#include "Interface.h"


#define MAX_QUEUE_SIZE 100

typedef struct {
    void* data;
    int dataSize;
} QueueElement;

typedef struct {
    QueueElement data[MAX_QUEUE_SIZE];
    int head;
    int tail;
    bool isSync;
} Queue;

Queue* createQueue(bool isSync) {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->head = 0;
    q->tail = 0;
    q->isSync = isSync;
    return q;
}

int isFull(Queue* q) {
    return (q->tail + 1) % MAX_QUEUE_SIZE == q->head;
}

int isEmpty(Queue* q) {
    return q->head == q->tail;
}

void enqueue(Queue* q, void* data, int dataSize) {
    if (q->isSync) {
        while (isFull(q)) {
            // prazno mesto
        }
    }
    else {
        if (isFull(q)) {
            //ako je red pun preskoci
            return;
        }
    }
    QueueElement element;
    element.data = data;
    element.dataSize = dataSize;
    q->data[q->tail] = element;
    q->tail = (q->tail + 1) % MAX_QUEUE_SIZE;
}

QueueElement dequeue(Queue* q) {
    if (q->isSync) {
        while (isEmpty(q)) {
            // čekaj dok nije dostupan element u redu
        }
    }
    else {
        if (isEmpty(q)) {
            // vrati praznu strukturu ako je red prazan
            QueueElement element;
            element.data = NULL;
            element.dataSize = 0;
            return element;
        }
    }
    QueueElement element = q->data[q->head];
    q->head = (q->head + 1) % MAX_QUEUE_SIZE;
    return element;
}

void SendData(bool isSync, void* data, int dataSize) {
    static Queue* q = NULL;
    if (q == NULL) {
        q = createQueue(isSync);
    }
    enqueue(q, data, dataSize);
}





//void SendData(int ServiceID, void* data, int dataSize) {	
    //return;
//}

void ReceiveData(bool isSync, void* data, int dataSize) {
    static Queue* q = NULL;
    if (q == NULL) {
        q = createQueue(isSync);
    }
    QueueElement element = dequeue(q);
    memcpy(data, element.data, dataSize);
}