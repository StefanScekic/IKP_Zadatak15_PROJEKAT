#pragma once

#ifndef REQUEST_INTERFACE_H
#define REQUEST_INTERFACE_H

#include "Includes.h"

#define MAX_DATA_SIZE 32768	//Maximum size of data part of requests sent by clients
#define MAX_FILE_NAME 32
#define MAX_FILE_SIZE (MAX_DATA_SIZE - MAX_FILE_NAME - sizeof(u_int) - sizeof(int))

typedef enum RequestCode
{
	RegisterService = 100,
	SendData = 101,
	Callback = 102,
	UnregisterService = 200
} request_code;

typedef struct Request {
	request_code code;
	char data[MAX_DATA_SIZE];
} request;

typedef struct process {
	int ID;
	SOCKET socket;
} process;

typedef struct File {
	int ownder_id;
	char file_name[MAX_FILE_NAME];
	char file_contents[MAX_FILE_SIZE];
	u_int file_length;
} file;

typedef struct service_interface {
	void (*register_service)(int service_id);
	void (*send_data)(int service_id, void *data, int data_size);
	void (*receive_data)(void* data, int data_size);
} service_interface;

#endif // !REQUEST_INTERFACE_H