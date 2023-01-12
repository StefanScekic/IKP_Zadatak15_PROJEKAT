#include "Connection.h"

bool InitializeWindowsSockets() {
	WSADATA wsa;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return false;
    }
	return true;
}