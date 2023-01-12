#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <WS2tcpip.h>
//Dodaje dependancy u podesavanje svakog projekta koji ovo includuje
#pragma comment(lib, "Ws2_32.lib")

/*
	Default server port used for testing.
*/
#define SERVERPORT 1800
/*
	Default server address, localhost.
*/
#define DEFAULT_ADDRESS "127.0.0.1"
/*
	Represents the maximum length of the queue of pending connections.
*/
#define SERVER_BACKLOG 20

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

/*
	Initializes WSAData.
	
	returns: true if successfull, otherwise false
 */
bool InitializeWindowsSockets();