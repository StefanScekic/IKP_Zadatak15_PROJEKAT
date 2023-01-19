#include "ClientSocket.h"

SOCKET receive_data_socket = INVALID_SOCKET; //Socket used for receiving data to be replicated
HANDLE receive_thread = NULL;                //Thread that runs recieve_data_socket
HANDLE rds_socket_finished = NULL;           //Semaphore that signals receive data socket is initialized

int rollbackCounter = 0; //Counter used for clean up

#pragma region SendRequestSocket
SOCKET send_request_socket = INVALID_SOCKET; //Socket used for sending requests to Replicator

SOCKET get_send_request_socket() {
    return send_request_socket;
}
#pragma endregion

#pragma region ProcessId
int process_id = 1;      //Process id, 1 by default

void set_process_id(int id) {
    process_id = id;
}

int get_process_id() {
    return process_id;
}
#pragma endregion

#pragma region ServerPort get/set
u_short server_port = DEFAULT_SERVER_PORT;

void set_server_port(u_short port) {
    server_port = port;
}

u_short get_server_port() {
    return server_port;
}
#pragma endregion

#pragma region ClientPort
u_short client_port = 1700;

void set_client_port(u_short port) {
    client_port = port;
}

u_short get_client_port() {
    return client_port;
}
#pragma endregion

#pragma region PrivateFuncionDescriptors

/*
 *Initializes a socket used for sending requests to the Replicator
 * 
 * @return 0 on success, -1 on failiure
 */
int init_request_socket();

DWORD WINAPI receive_thread_function(LPVOID arg);

#pragma endregion

#pragma region PublicFunctions

void init_client_sockets(){
    //WSA init
    if (InitializeWindowsSockets() == FALSE)
        cleanup(WSA_FAIL);
    
    rollbackCounter = 1;

    //Initialize semaphore
    if ((rds_socket_finished = CreateSemaphore(NULL, 0, 1, NULL)) == NULL) {
        printf_s("CreateSemaphore failed with error code: %d\n", GetLastError());
        cleanup(SMF_FAIL);
    }

    rollbackCounter = 2;

    //Initialize the socket used for sending request
    if ((init_request_socket()) < 0) {
        cleanup(SC_FAIL);
    }

    rollbackCounter = 3;

    //Connect to the Replicator
    //init the server address struct
    SA_IN server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(DEFAULT_ADDRESS);
    server_addr.sin_port = htons(server_port);

    if (connect(send_request_socket, (SA*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        printf_s("Connect failed with error code: %d\n", WSAGetLastError());
        cleanup(CON_FAIL);
    }

    rollbackCounter = 4;

    //Start the socket that listens for incoming data
    if ((receive_thread = CreateThread(NULL, 0, receive_thread_function, NULL, 0, 0)) == NULL) {
        printf_s("CreateThread failed with error code: %d\n", GetLastError());
        cleanup(TH_FAIL);
    }

    WaitForSingleObject(rds_socket_finished, INFINITE);

    rollbackCounter = 5;

    return;
}

void cleanup(int exit_code) {
    printf_s("Cleanup started\n");

    switch (rollbackCounter)
    {
    case 5:
        //Close receive socket and free thread handle
        closesocket(receive_data_socket);
        CloseHandle(receive_thread);
    case 4:
        //Send request for process to be removed from registered processes struct
        send_request(UnregisterService);
    case 3:
        //Close socket for sending requests
        closesocket(send_request_socket);
    case 2:
        //Release semaphore handles
        CloseHandle(rds_socket_finished);
    case 1:
        //WindowsSockets cleanup
        WSACleanup();
    case 0:
        if (exit_code != ALL_GOOD) {
            printf_s("\nExiting process with exit code : %s\n", get_exit_code_name(exit_code));
            exit(exit_code);
        }
        break;
    default:
        printf_s("Rollback count not handled\n");
    }

    return;
}

void send_request(request_code code) {

    //DO SOMETHING
    switch (code)
    {
    case SendData:

        break;
    case RegisterService:
        {}
        process p;
        struct sockaddr_in socket_address;
        int socket_address_len = sizeof(socket_address);

        getsockname(receive_data_socket, (struct sockaddr*)&socket_address, &socket_address_len);
    
        u_short port = ntohs(socket_address.sin_port);

        p.ID = process_id;
        p.address = inet_addr(DEFAULT_ADDRESS);
        p.port = port;

        process_service.register_service(p);    
        break;
    case ReceiveData:

        break;
    case UnregisterService:
        unregister_service(process_id);

        break;
    default:

        break;
    }

    return;
}

#pragma endregion

#pragma region PrivateFunctions

int init_request_socket() {    
    send_request_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (send_request_socket == INVALID_SOCKET) {
        printf_s("send_request_socket creation failed, error code : %d", WSAGetLastError());
        return -1;
    }

    return 0;
}

DWORD WINAPI receive_thread_function(LPVOID arg) {
    int iResult = 0;
    char buffer[DEFAULT_BUFLEN];

    SA_IN server_addr;

    SOCKET accepted_socket = INVALID_SOCKET;

    //Server socket creation
    receive_data_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (receive_data_socket == INVALID_SOCKET) {
        printf_s("Client listen socket creation failed, error code : %d", WSAGetLastError());
        return 1;
    }

    //init the address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(DEFAULT_ADDRESS);
    server_addr.sin_port = htons(client_port);

    //Bind socket
    bind(receive_data_socket, (SA*)&server_addr, sizeof(server_addr));
    if (receive_data_socket == SOCKET_ERROR) {
        printf_s("Client listen socket binding failed, error code : %d", WSAGetLastError());
        return 2;
    }

    //Set socket to listen mode
    listen(receive_data_socket, 1);
    if (receive_data_socket == SOCKET_ERROR) {
        printf_s("Client listen socket listen mode failed, error code : %d", WSAGetLastError());
        return 3;
    }

    printf_s("Socket listening on port %d\n", client_port);
    ReleaseSemaphore(rds_socket_finished, 1, NULL);     //Tell process that it's safe to continue init

    //Accept connections and rcv data
    do
    {
        accepted_socket = accept(receive_data_socket, NULL, NULL);

        if (accepted_socket == INVALID_SOCKET)
        {
            printf("Accept failed with error: %d\n", WSAGetLastError());
            return 4;
        }

        do
        {
            // Receive data until Replicator shuts down the connection
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
                return 5;
            }
        } while (iResult > 0);

    } while (TRUE);

    return 0;
}

#pragma endregion