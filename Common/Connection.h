#pragma once

#ifndef CONNECTION_H
#define CONNECTION_H

#include "Includes.h"

/*
	Default server port used for testing.
*/
#define DEFAULT_SERVER_PORT 1800
/*
	Default server address, localhost.
 */
#define DEFAULT_ADDRESS "127.0.0.1"
/*
	Default size of buffers used for sending/receiving data
 */
#define DEFAULT_BUFLEN 4096

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

/*
	Initializes WSAData.
	
	@returns TRUE if successfull, otherwise FALSE
 */
BOOL InitializeWindowsSockets();

#endif // !CONNECTION_H