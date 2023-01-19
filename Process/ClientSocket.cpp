#include "ClientSocket.h"
#include "ProcessService.h"

SOCKET send_request_socket = INVALID_SOCKET; //Socket used for sending requests to Replicator
SOCKET receive_data_socket = INVALID_SOCKET; //Socket used for receiving data to be replicated

HANDLE receive_thread;

int rollbackCounter = 0; //Counter used for clean up

DWORD WINAPI receive_thread_function(LPVOID arg) {
    int iResult = 0;
    char buffer[DEFAULT_BUFLEN];

    SA_IN server_addr;
    u_short cp = *((u_short*)arg);

    SOCKET accepted_socket = INVALID_SOCKET;

    //Server socket creation
    receive_data_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (receive_data_socket == INVALID_SOCKET) {
        printf_s("Client listen socket creation failed, error code : %d", WSAGetLastError());
        return -1;
    }

    //init the address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(DEFAULT_ADDRESS);
    server_addr.sin_port = htons(cp);

    //Bind socket
    bind(receive_data_socket, (SA*)&server_addr, sizeof(server_addr));
    if (receive_data_socket == SOCKET_ERROR) {
        printf_s("Client listen socket binding failed, error code : %d", WSAGetLastError());
        return -1;
    }

    //Set socket to listen mode
    listen(receive_data_socket, 1);
    if (receive_data_socket == SOCKET_ERROR) {
        printf_s("Client listen socket listen mode failed, error code : %d", WSAGetLastError());
        return -1;
    }

    printf_s("Socket listening on port %d\n", cp);

    //Accept connections and rcv data
    do
    {
        accepted_socket = accept(receive_data_socket, NULL, NULL);

        if (accepted_socket == INVALID_SOCKET)
        {
            printf("accept failed with error: %d\n", WSAGetLastError());
            return -1;
        }

        do
        {
            // Receive data until the client shuts down the connection
            iResult = recv(accepted_socket, buffer, sizeof(buffer), 0);
            if (iResult > 0)
            {
                buffer[iResult] = '\0';
                printf("Message received from client: %s.\n", buffer);
            }
            else if (iResult == 0)
            {
                // connection was closed gracefully
                printf("Connection with client closed.\n");
                closesocket(accepted_socket);
                break;
            }
            else
            {
                // there was an error during recv
                printf("recv failed with error: %d\n", WSAGetLastError());
                closesocket(accepted_socket);
                return -1;
            }
        } while (iResult > 0);

    } while (1);

    return 0;
}

/*
 *Initializes a socket used for sending requests to the Replicator
 * 
 * @return 0 on success, -1 on failiure
 */
int init_request_socket() {    
    send_request_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (send_request_socket == INVALID_SOCKET) {
        printf_s("send_request_socket creation failed, error code : %d", WSAGetLastError());
        return -1;
    }

    return 0;
}

/*
 * Initializes all sockets and resources used by Process
 * 
 * @param client_port: Port on which Process will listen for incoming Data
 */
void init_client_sockets(u_short *client_port) {
    //WSA init
    if (InitializeWindowsSockets() == false)
        cleanup(WSA_FAIL);

    rollbackCounter = 1;

    //Initialize the socket used for sending request
    if ((init_request_socket()) < 0) {
        cleanup(RSC_FAIL);
    }

    rollbackCounter = 2;

    //Connect to the Replicator
    //init the server address struct
    SA_IN server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(DEFAULT_ADDRESS);
    server_addr.sin_port = htons(SERVERPORT);                   //TODO dodati podrsku odabira porta replikatora

    if (connect(send_request_socket, (SA*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        handle_send_request_result(CON_FAIL);
        cleanup(-4);
    }

    rollbackCounter = 3;

    //Start the socket that listens for incoming data
    if ((receive_thread = CreateThread(NULL, 0, receive_thread_function, client_port, 0, 0)) == NULL) {
        printf_s("CreateThread failed with error code: %d\n", GetLastError());
        cleanup(-4);
    }

    rollbackCounter = 4;

    return;
}

void send_request(int server_port, RequestCode code) {
    

    //DO SOMETHING
    switch (code)
    {
    case RegisterService:
        process p;
        p.ID = 1;
        p.address = inet_addr(DEFAULT_ADDRESS);
        p.port = htons(8080);

        process_service.register_service(p);    
        break;
    case SendData:

        break;
    case ReceiveData:

        break;
    case UnregisterService:
        unregister_service();

        break;
    default:

        break;
    }

    handle_send_request_result(SUC);
    return;
}

void handle_send_request_result(SEND_REQUEST_RESULT result) {
    switch (result)
    {
    case SUC:
        printf_s("Request successfully sent.\n");
        break;
    case NO_RESPONSE:
        printf_s("recv failed with error: %d\n", WSAGetLastError());
        break;
    case SEND_FAIL:
        printf_s("Send failed with error: %d\n", WSAGetLastError());
        break;
    case CON_FAIL:
        printf_s("Unable to connect to server, error: %d\n", WSAGetLastError());
        break;
    default:
        printf_s("Unhandled request result detected.\n");
        break;
    }
}

void cleanup(int exit_code) {
    printf_s("Cleanup started\n");

    switch (rollbackCounter)
    {
    case 4:
        closesocket(receive_data_socket);
        CloseHandle(receive_thread);
    case 3:
        send_request(SERVERPORT ,UnregisterService);
    case 2:
        //RequestSocket cleanup
        closesocket(send_request_socket);
    case 1:
        //WSA cleanup
        WSACleanup();
    case 0:
        if (exit_code != ALL_GOOD) {
            printf_s("\nExiting ClientSocket with exit code : %d\n", exit_code);
            exit(exit_code);
        }
    }

    return;
}