#pragma once
#include "Includes.h"

#define MESSAGE_DATA_LENGTH 4096

enum RequestCode
{
	RegisterService = 100,
	SendData = 101,
	ReceiveData = 102
};

typedef struct Request {
	RequestCode code;
	char *data;
	u_int data_size;
} request;

typedef struct process {
	int ID;
	u_long address;
	u_short port;
} process;

typedef struct service_interface {
	void (*register_service)(int service_id);
	void (*send_data)(int service_id, void *data, int data_size);
	void (*receive_data)(void* data, int data_size);
} service_interface;