#pragma once
#include "../Common/Includes.h"
#include "../Common/Connection.h"
#include "../Common/RequestInterface.h"

#ifndef CLIENT_SOKCET_H
#define CLIENT_SOKCET_H

extern SOCKET send_request_socket;


enum ExitCode
{
	ALL_GOOD = 0,	//yay
	WSA_FAIL = -1,	//WSA fail
	RSC_FAIL = -2,	//Request socket creation failed
};

enum SEND_REQUEST_RESULT {
	SUC = 0,			//yay
	CON_FAIL = -1,		//Connecting to server failed
	SEND_FAIL = -2,		//Sending request to server failed
	NO_RESPONSE = -3
};

void init_client_sockets(int client_port);

void send_request(int server_port, RequestCode code);

void handle_send_request_result(SEND_REQUEST_RESULT result);

void cleanup(int exit_code);


#endif // !client_socket_H