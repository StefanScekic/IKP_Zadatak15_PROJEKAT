#pragma once
#include "../Common/Includes.h"

#ifndef SQUEUE_H
#define SQUEUE_H

struct node {
	struct node* next;
	SOCKET* client_socket;
};
typedef struct node node_t;

void enqueue(SOCKET* client_socket);

/*
	Returns NULL if the queue is empty.
	Returns the pointer to a client_socket, if there is one to get.
*/
SOCKET* dequeue();

#endif // !QUEUE_H
