#pragma once

#ifndef CLIENT_SOKCET_H
#define CLIENT_SOKCET_H

#include "../Common/Includes.h"
#include "../Common/Connection.h"
#include "../Common/RequestInterface.h"
#include "../Common/Utils.h"
#include "ProcessService.h"

typedef enum SEND_REQUEST_RESULT {
	SUC = 0,			//yay
	CON_FAIL = -1,		//Connecting to server failed
	SEND_FAIL = -2,		//Sending request to server failed
	NO_RESPONSE = -3
} send_request_result;

#pragma region GetSet
//Get, Set for server_port
void set_server_port(u_short port);
u_short get_server_port();

//Get, Set for process_id
void set_process_id(int id);
int get_process_id();

//Get send_request_socket
SOCKET get_send_request_socket();

SOCKET get_receive_data_socket();

char* get_process_dir();
#pragma endregion

//Rest of functions

/*
	Initializes all sockets and resources used by Process
 */
void init_client_sockets();

void cleanup(int exit_code);

void send_request(request_code code, void* data);

#endif // !client_socket_H