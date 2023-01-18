#pragma once
#include "../Common/Connection.h"
#include "../Common/RequestInterface.h"
#include "SQueue.h"
#include "ReplicationService.h"

#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#define THREAD_POOL_SIZE 20
/*
	Represents the maximum length of the queue of pending connections.
*/
#define SERVER_BACKLOG 200

extern HANDLE sinterrupt_main;

void boot_server_socket(int server_port);
void cleanup(int exit_code);

enum ExitCode
{
	ALL_GOOD = 0,	//yay
	WSA_FAIL = -1,	//WSA fail
	SMF_FAIL = -3,	//Semaphore fail
	TP_FAIL = -4,	//ThreadPool fail
	SS_FAIL = -5,	//ServerSocket fail
	MT_FAIL = -6,	//MainServerThread fail
	HC_FAIL = -7	//HandleConnection function fail
};
#endif // !SERVER_SOCKET_H