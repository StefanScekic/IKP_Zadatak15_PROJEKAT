#include <stdio.h>
#include "../Common/Connection.h"

#define DEFAULT_BUFLEN 512

DWORD WINAPI handle_connection(LPVOID client_socket);

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
            //printf("%d\n\n", client_socket); //DEBUGG
        }

        //Handle the connection
        SOCKET *pclient = (SOCKET*)malloc(sizeof(SOCKET));
        *pclient = client_socket;
        HANDLE t;
        t = CreateThread(NULL, 0, &handle_connection, pclient, 0, 0);
        //handle_connection(pclient); //used for thread testing

    } //while

    //Clean-up
    closesocket(server_socket);
    WSACleanup();
    return 0;
}

DWORD WINAPI handle_connection(LPVOID client_socket) {
    int iResult;
    int msgSize = 0;
    char recvbuf[DEFAULT_BUFLEN];

    SOCKET cs = *((SOCKET*)client_socket);
    free(client_socket);

    while ((iResult = recv(cs, recvbuf + msgSize, sizeof(recvbuf) - msgSize - 1, 0)) > 0) {
        msgSize += iResult;
        if (msgSize > DEFAULT_BUFLEN - 1 || recvbuf[msgSize - 1] == '\n')
            break;
    }

    if (iResult < 0) {
        printf("recv failed with error: %d\n", WSAGetLastError());
        closesocket(cs);
    }
    recvbuf[msgSize - 1] = '\0';

    printf("REQUEST: %s\n", recvbuf);
    _flushall();

    //Sleep(1000); //Used for thread testing purposes

    const char* odgovor = "odgovor";
    iResult = send(cs, odgovor, strlen(odgovor), 0);

    if (closesocket(cs) == SOCKET_ERROR)
        printf("Close socket failed with error : %d", WSAGetLastError());

    printf("Connection with client closed.\n");

    return 0;
}