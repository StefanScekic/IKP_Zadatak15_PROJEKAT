#pragma once

#ifndef REQUEST_INTERFACE_H
#define REQUEST_INTERFACE_H

#include "Includes.h"

#define MAX_DATA_SIZE 2048	//Maximum size of data part of requests sent by clients

typedef enum RequestCode
{
	RegisterService = 100,
	SendData = 101,
	ReceiveData = 102,
	UnregisterService = 200
} request_code;

typedef struct Request {
	request_code code;
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

#endif // !REQUEST_INTERFACE_H