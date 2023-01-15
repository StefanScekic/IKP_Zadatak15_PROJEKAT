#pragma once
#include "Includes.h"

#define MESSAGE_DATA_LENGTH 4096

enum RequestCode
{
	RegisterService = 100,
	SendData = 101
};

typedef struct Request {
	RequestCode code;
	char data[MESSAGE_DATA_LENGTH];
} REQUEST;

void register_service(int service_id);

void send_data(int service_id, void* data, int data_size);