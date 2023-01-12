#include <stdio.h>
#include "../Common/Connection.h"

#define DEFAULT_BUFLEN 512

void handle_connection(SOCKET *client_socket);

int main() {
    if (InitializeWindowsSockets() == false)
    {
        return 1;
    }

    SOCKET server_socket = INVALID_SOCKET;
    SOCKET client_socket = INVALID_SOCKET;
    SA_IN server_addr, client_addr;
    int addr_size;
    
    //Server socket creation
    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET) {
        printf("Server socket creation failed, error code : %d", WSAGetLastError());

        WSACleanup();
        return 1; 
    }    

    //init the address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(DEFAULT_ADDRESS);
    server_addr.sin_port = htons(SERVERPORT);

    //Bind socket
    bind(server_socket, (SA*)&server_addr, sizeof(server_addr));
    if (server_socket == SOCKET_ERROR) {
        printf("Server socket binding failed, error code : %d", WSAGetLastError());

        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    //Set socket to listen mode
    listen(server_socket, SERVER_BACKLOG);
    if (server_socket == SOCKET_ERROR) {
        printf("Server socket listen mode failed, error code : %d", WSAGetLastError());

        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    while (true) {
        printf("Waiting for connections...\n");

        //Wait and accept an incoming connection
        //Addr_size changes with each accept, so a reset is needed
        addr_size = sizeof(SA_IN);

        client_socket = accept(server_socket, (SA*)&client_addr, (socklen_t*)&addr_size);
        if (client_socket == INVALID_SOCKET) {
            printf("Client socket accept failed, error code : %d", WSAGetLastError());

            closesocket(server_socket);
            WSACleanup();
            return 1;
        }
        else {
            printf("Connected!\n");
        }

        //Handle the connection
        handle_connection(&client_socket);

    } //while

    //Clean-up
    closesocket(server_socket);
    WSACleanup();
    return 0;
}

void handle_connection(SOCKET *client_socket) {
    int iResult;
    char recvbuf[DEFAULT_BUFLEN];

    do
    {
        // Receive data until the client shuts down the connection
        iResult = recv(*client_socket, recvbuf, DEFAULT_BUFLEN, 0);
        if (iResult > 0)
        {
            printf("Message received from client: %s.\n", recvbuf);
        }
        else if (iResult == 0)
        {
            // connection was closed gracefully
            printf("Connection with client closed.\n");
            closesocket(*client_socket);
        }
        else
        {
            // there was an error during recv
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(*client_socket);
        }
    } while (iResult > 0);

    return;
}