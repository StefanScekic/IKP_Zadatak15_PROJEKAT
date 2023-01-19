#include "Connection.h"

BOOL InitializeWindowsSockets() {
	WSADATA wsa;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf_s("WSAStartup failed: %d\n", WSAGetLastError());
        return FALSE;
    }
	return TRUE;
}