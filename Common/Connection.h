#include "Includes.h"

/*
	Default server port used for testing.
*/
#define SERVERPORT 1800
/*
	Default server address, localhost.
*/
#define DEFAULT_ADDRESS "127.0.0.1"

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

/*
	Initializes WSAData.
	
	returns: true if successfull, otherwise false
 */
bool InitializeWindowsSockets();