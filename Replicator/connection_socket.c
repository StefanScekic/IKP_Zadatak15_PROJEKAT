#include <stdio.h>
#include <winsock2.h> // For socket functions

int main() {
    WSADATA wsa; // Structure to store winsock information

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }

    // Create a socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        printf("Failed to create socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Fill in the sockaddr_in structure
    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); // localhost
    server.sin_family = AF_INET;
    server.sin_port = htons(80); // port 80

    // Connect to the server
    if (connect(sock, (struct sockaddr *) &server, sizeof(server)) == SOCKET_ERROR) {
        printf("Failed to connect: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Send a message to the server
    char message[] = "Hello, server!";
    if (send(sock, message, sizeof(message), 0) == SOCKET_ERROR) {
        printf("Failed to send message: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Close the socket and cleanup Winsock
    closesocket(sock);
    WSACleanup();

    return 0;
}