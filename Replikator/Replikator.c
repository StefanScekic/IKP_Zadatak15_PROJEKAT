#include <stdio.h>
#include "replication_service.h"
#include "queue.h"

int main() {
    // Poziv funkcija iz interfejsa
   /* int dataSize = 5;
    void* data = &dataSize;

    ReceiveData(data, dataSize);*/
    Queue* q = createQueue();

    enqueue(q, 1);
    enqueue(q, 2);
    enqueue(q, 3);

    printf("%d ", dequeue(q));
    printf("%d ", dequeue(q));
    printf("%d\n", dequeue(q));

    return 0;
}