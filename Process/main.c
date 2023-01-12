#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <WS2tcpip.h>

int check(int exp, const char* msg);

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

#define SERVERPORT 1800

int main() {
    WSADATA wsa;
    int iResult;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }

    SOCKET client_socket = INVALID_SOCKET;
    check(
        (client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)),
        "Failed to create socket"
    );

    //init the address struct
    SA_IN server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(SERVERPORT);

    // connect to server specified in serverAddress and socket connectSocket
    if (connect(client_socket, (SA*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        printf("Unable to connect to server.\n");
        closesocket(client_socket);
        WSACleanup();
    }

    char* messageToSend = "kek lmao";
    iResult = send(client_socket, messageToSend, (int)strlen(messageToSend) + 1, 0);
    if (iResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    printf("Bytes Sent: %ld\n", iResult);

    closesocket(client_socket);
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