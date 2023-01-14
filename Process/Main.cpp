#include <stdio.h>
#include "../Common/Connection.h"

#define SERVERPORT 1800

int main() {
    int iResult;

    if (InitializeWindowsSockets() == false)
    {
        return 1;
    }

    SOCKET client_socket = INVALID_SOCKET;

    //Client socket creation
    client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(client_socket == INVALID_SOCKET) {
        printf_s("Client socket creation failed, error code : %d", WSAGetLastError());

        WSACleanup();
        return 1;
    }

    //init the server address struct
    SA_IN server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(DEFAULT_ADDRESS);
    server_addr.sin_port = htons(SERVERPORT);

    // connect to server specified in serverAddress and socket connectSocket
    if (connect(client_socket, (SA*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        printf_s("Unable to connect to server.\n");

        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    //DO SOMETHING
    const char* messageToSend = "kek lmao";
    iResult = send(client_socket, messageToSend, (int)strlen(messageToSend) + 1, 0);
    if (iResult == SOCKET_ERROR)
    {
        printf_s("send failed with error: %d\n", WSAGetLastError());

        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    printf_s("Bytes Sent: %ld\n", iResult);

    //Clean-up
    closesocket(client_socket);
    WSACleanup();

    getchar();
    return 0;
}