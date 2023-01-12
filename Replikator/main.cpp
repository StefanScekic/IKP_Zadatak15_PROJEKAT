#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <WS2tcpip.h>
#include "replication_service.h"
#include "queue.h"

#define SERVERPORT 1800
#define SERVER_BACKLOG 20
#define BUFSIZE 4096
#define DEFAULT_BUFLEN 512

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

int check(int exp, const char* msg);
void handle_connection(SOCKET client_socket);

int main() {
    WSADATA wsa;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }

    SOCKET server_socket = INVALID_SOCKET, client_socket = INVALID_SOCKET;
    SA_IN server_addr, client_addr;
    int addr_size;

    check(
        (server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)),
        "Failed to create socket"
    );

    //init the address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(SERVERPORT);

    check(
        bind(server_socket, (SA*)&server_addr, sizeof(server_addr)),
        "Bind Failed"
    );

    check(
        listen(server_socket, SERVER_BACKLOG),
        "Listen Failed!"
    );

    while (true) {
        printf("Waiting for connections...\n");

        //Wait and accept an incoming connection
        addr_size = sizeof(SA_IN);
        check(
            client_socket = accept(server_socket, (SA*)&client_addr, (socklen_t*)&addr_size),
            "Accept failed"
        );
        printf("Connected!\n");

        //Handle the connection
        handle_connection(client_socket);

    } //while

    closesocket(server_socket);
    WSACleanup();
    return 0;
}

int check(int exp, const char* msg) {
    if (exp == SOCKET_ERROR) {
        printf("%s : %d", msg, WSAGetLastError());

        WSACleanup();
        exit(1);
    }
}

void handle_connection(SOCKET client_socket) {
    int iResult;
    char recvbuf[DEFAULT_BUFLEN];

    do
    {
        // Receive data until the client shuts down the connection
        iResult = recv(client_socket, recvbuf, DEFAULT_BUFLEN, 0);
        if (iResult > 0)
        {
            printf("Message received from client: %s.\n", recvbuf);
        }
        else if (iResult == 0)
        {
            // connection was closed gracefully
            printf("Connection with client closed.\n");
            closesocket(client_socket);
        }
        else
        {
            // there was an error during recv
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(client_socket);
        }
    } while (iResult > 0);

    return;
}