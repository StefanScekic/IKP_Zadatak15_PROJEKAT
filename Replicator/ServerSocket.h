#pragma once
#include "../Common/Connection.h"
#include "../Common/RequestInterface.h"
#include "../Common/Utils.h"
#include "SQueue.h"
#include "ReplicationService.h"

#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#define DEFAULT_REPLICATOR_CONNECT_PORT	9999	//Port of another replicator
#define THREAD_POOL_SIZE 20						//Number of woker threads
#define SERVER_BACKLOG 200						//Number of sockets that can wait for connection

extern HANDLE sinterrupt_main;

#pragma region GetSets

void set_server_port(u_short port);
u_short get_server_port();

void set_replicator_port(u_short port);
u_short get_replicator_port();

SOCKET get_rtr_socket();
#pragma endregion


void boot_server_socket();
void cleanup(int exit_code);
#endif // !SERVER_SOCKET_H