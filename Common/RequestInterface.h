#pragma once
#include "Includes.h"

#define MESSAGE_DATA_LENGTH 4096
#define MAX_DATA_SIZE 2048

enum RequestCode
{
	RegisterService = 100,
	SendData = 101,
	ReceiveData = 102,
	UnregisterService = 200
};

typedef struct Request {
	RequestCode code;
	char data[MAX_DATA_SIZE];
} request;

typedef struct process {
	int ID;
	u_long address;
	u_short port;
} process;

typedef struct service_interface {
	void (*register_service)(process p);
	void (*send_data)(int service_id, void *data, int data_size);
	void (*receive_data)(void* data, int data_size);
} service_interface;