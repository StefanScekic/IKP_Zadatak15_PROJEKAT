#include "ClientSocket.h"

HANDLE receive_thread = NULL;                //Thread that runs recieve_data_socket
HANDLE rds_socket_finished = NULL;           //Semaphore that signals receive data socket is initialized

int rollbackCounter = 0; //Counter used for clean up

#pragma region ReceiveDataSocket
SOCKET receive_data_socket = INVALID_SOCKET; //Socket used for receiving data to be replicated

SOCKET get_receive_data_socket() {
    return receive_data_socket;
}
#pragma endregion

#pragma region SendRequestSocket
SOCKET send_request_socket = INVALID_SOCKET; //Socket used for sending requests to Replicator

SOCKET get_send_request_socket() {
    return send_request_socket;
}
#pragma endregion

#pragma region ProcessDirectory

char resource_directory[100] = "process";

char* get_process_dir() {
    return resource_directory;
}
#pragma endregion

#pragma region ProcessId
int process_id = 1;      //Process id, 1 by default

void set_process_id(int id) {
    process_id = id;

    char index_string[10];
    _itoa(process_id, index_string, 10);
    strcat(resource_directory, index_string);
    strcat(resource_directory, "_resources");
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
        WaitForSingleObject(receive_thread, 2000);
        CloseHandle(receive_thread);
    case 4:
        //Send request for process to be removed from registered processes struct
        send_request(UnregisterService, NULL);
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

void send_request(request_code code, void *data) {

    //DO SOMETHING
    switch (code)
    {
    case SendData:
        {}
        process_service.send_data(process_id, data, 1);
        break;
    case RegisterService:
        process_service.register_service(process_id);    
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
    char buffer[MAX_DATA_SIZE];

    SA_IN server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(DEFAULT_ADDRESS);
    server_addr.sin_port = htons(server_port);

    SOCKET accepted_socket = INVALID_SOCKET;

    //Server socket creation
    receive_data_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (receive_data_socket == INVALID_SOCKET) {
        printf_s("Client listen socket creation failed, error code : %d", WSAGetLastError());
        return 1;
    }

    if (connect(receive_data_socket, (SA*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        printf_s("Connect failed with error code: %d\n", WSAGetLastError());
    }

    ReleaseSemaphore(rds_socket_finished, 1, NULL);     //Tell process that it's safe to continue init

    //Accept connections and rcv data
    do
    {
        printf("Connected!\n");
        do
        {
            // Receive data until Replicator shuts down the connection
            iResult = recv(receive_data_socket, buffer, sizeof(buffer), 0);
            if (iResult > 0)
            {
                if (iResult < 100) {
                    buffer[iResult] = '\0';
                    printf("Message received from replicator: %s\n", buffer);

                }
                else
                {
                    file temp_file;
                    memcpy(&temp_file, buffer, sizeof(file));

                    char wr_location[100] = "";
                    char file_name[MAX_FILE_NAME] = { 0 };

                    memcpy(file_name, temp_file.file_name, sizeof(temp_file.file_name));
                    strtok(file_name, "Ì");

                    strcat(wr_location, get_process_dir());
                    strcat(wr_location, "\\");
                    strcat(wr_location, file_name);

                    printf("%s\n", wr_location);

                    write_file(wr_location, temp_file.file_contents, temp_file.file_length - 4);
                }
            }
            else if (iResult == 0)
            {
                // connection was closed gracefully
                printf("Connection with client closed.\n");
                closesocket(accepted_socket);
                return 0;
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