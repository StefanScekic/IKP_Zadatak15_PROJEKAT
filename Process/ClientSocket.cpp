#include "ClientSocket.h"
#include "ProcessService.h"

SOCKET send_request_socket = INVALID_SOCKET; //Socket used for sending requests to Replicator
int rollbackCounter = 0; //Counter used for clean up

int init_request_socket() {
    //Client socket creation
    send_request_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (send_request_socket == INVALID_SOCKET) {
        printf_s("send_request_socket creation failed, error code : %d", WSAGetLastError());
        return -1;
    }

    return 0;
}

void init_client_sockets(int client_port) {
    //WSA init
    if (InitializeWindowsSockets() == false)
        cleanup(WSA_FAIL);

    rollbackCounter = 1;

    if ((init_request_socket()) < 0) {
        cleanup(RSC_FAIL);
    }

    rollbackCounter = 2;

    //init the server address struct
    SA_IN server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(DEFAULT_ADDRESS);
    server_addr.sin_port = htons(SERVERPORT);

    // connect to server specified in serverAddress and socket connectSocket
    if (connect(send_request_socket, (SA*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        handle_send_request_result(CON_FAIL);
        return;
    }
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
    case 2:
        //RequestSocket cleanup
        send_request(SERVERPORT ,UnregisterService);
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