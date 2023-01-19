#include "ServerSocket.h"

HANDLE threadPool[THREAD_POOL_SIZE];        //Thread pool

SOCKET server_socket = INVALID_SOCKET;      //Accept requests socket
CRITICAL_SECTION cs;                 //Enqueue/Dequeue critical section
CRITICAL_SECTION cs_hash_table;      //HashTable critical section
fd_set current_sockets;              //FDs of socekts select is listening on
fd_set ready_sockets;                //Helper FD set, select is destructive

HANDLE sinterrupt_main = NULL;              //End program semaphore
HANDLE semafore = NULL;                     //There is work semaphore
HANDLE interrupt_semaphore = NULL;          //Close worker threads semaphore
HANDLE as_thread = NULL;                    //Accept requests socket thread

int counter = 0;                            //Counts the number of connections handled
int rollbackCounter = 0;                    //Counter used for clean up

#pragma region ServerPort
u_int server_port = DEFAULT_SERVER_PORT;

void set_server_port(u_short port) {
    server_port = port;
}

u_short get_server_port() {
    return server_port;
}
#pragma endregion

#pragma region ReplicatorPort
u_int replicator_port = 9999;       //Port for server to conect to another replicator

void set_replicator_port(u_short port) {
    replicator_port = port;
}

u_short get_replicator_port() {
    return replicator_port;
}
#pragma endregion

#pragma region PrivateFunctionDescriptors

/*
    Close all sockets saved in FDs
*/
void close_sockets();
/*
    Initialize ThreadPool
*/
int init_tp();
/*
    Function ran by thread that accpts requests from clients
*/
DWORD WINAPI accept_connections_thread_function(LPVOID arg);
/*
    Function ran by worker threads that handle requests
*/
DWORD WINAPI thread_function(LPVOID arg);
/*
    Function called by worker threads to process requests
*/
int handle_connection(SOCKET* client_socket);
/*
    Loads id from data and calls hash_table_delete
*/
void handle_unregister_service(char* data);
/*
    Initializes server socket params and puts it in listen mode
*/
SOCKET server_setup();
#pragma endregion

#pragma region PublicFunctions

void cleanup(int exit_code) {
    int i = 0;
    printf_s("Cleanup started\n");

    switch (rollbackCounter)
    {
    case 7:
        //Do nothing
    case 6:
        //HashTable Cleanup
        delete_hash_table();
    case 5:
        //Sockets cleanup
        close_sockets();
        CloseHandle(as_thread);
    case 4:
        //ThreadPool cleanup
        if (!ReleaseSemaphore(interrupt_semaphore, THREAD_POOL_SIZE, NULL)) {
            printf_s("ReleaseSemaphore error: %d\n", GetLastError());
        }

        WaitForMultipleObjects(THREAD_POOL_SIZE, threadPool, TRUE, INFINITE); //Wait for threads to finish their work
        for (i = 0; i < THREAD_POOL_SIZE; i++) {
            CloseHandle(threadPool[i]);
        }
    case 3:
        //Semaphore cleanup
        CloseHandle(semafore);
        CloseHandle(interrupt_semaphore);
        CloseHandle(sinterrupt_main);
    case 2:
        //CriticalSection cleanup
        DeleteCriticalSection(&cs);
        DeleteCriticalSection(&cs_hash_table);
    case 1:
        //WSA cleanup
        WSACleanup();
    case 0:
        if (exit_code != ALL_GOOD) {
            printf_s("\nExiting ServerAcceptSocket with exit code : %s\n", get_exit_code_name(exit_code));
            exit(exit_code);
        }
    }    

    printf_s("\n%d\n", counter);
    return;
}

void boot_server_socket() {
    //WindowsSocket init
    if (InitializeWindowsSockets() == FALSE)
        cleanup(WSA_FAIL);

    rollbackCounter = 1;

    //CriticalSection init
    InitializeCriticalSection(&cs);
    InitializeCriticalSection(&cs_hash_table);

    rollbackCounter = 2;

    //Semaphore init
    if ((semafore = CreateSemaphore(NULL, 0, 4096, NULL)) == NULL)
        cleanup(SMF_FAIL);
    if ((interrupt_semaphore = CreateSemaphore(NULL, 0, THREAD_POOL_SIZE, NULL)) == NULL)
        cleanup(SMF_FAIL);
    if ((sinterrupt_main = CreateSemaphore(NULL, 0, 1, NULL)) == NULL)
        cleanup(SMF_FAIL);

    rollbackCounter = 3;

    //ThreadPool init
    if ((init_tp()) != 0)
        cleanup(TP_FAIL);

    rollbackCounter = 4;

    //ServerSocket init
    FD_ZERO(&current_sockets);

    server_socket = server_setup();
    if ((server_socket == INVALID_SOCKET) || (server_socket == SOCKET_ERROR))
        cleanup(SC_FAIL);

    FD_SET(server_socket, &current_sockets);

    rollbackCounter = 5;

    //HashTable init
    init_hash_table();

    rollbackCounter = 6;

    //Main Accept incoming connections thread
    if ((as_thread = CreateThread(NULL, 0, accept_connections_thread_function, NULL, 0, 0)) == NULL) {
        printf_s("CreateThread failed with error code: %d\n", GetLastError());
        cleanup(TH_FAIL);
    }

    rollbackCounter = 7;

    return;
}

#pragma endregion

#pragma region PrivateFunctions

void close_sockets() {
    int i = 0;

    for (i = 0; i < current_sockets.fd_count; i++) {
        closesocket(current_sockets.fd_array[i]);
    }
}

int init_tp() {
    int i = 0;

    //Create the thread pool    
    for (i = 0; i < THREAD_POOL_SIZE; i++) {
        threadPool[i] = CreateThread(NULL, 0, thread_function, NULL, 0, 0);
        if (threadPool[i] == NULL) {
            printf_s("CreateThread failed with error code: %d\n", GetLastError());
            return -1;
        }
    }

    return 0;
}

DWORD WINAPI accept_connections_thread_function(LPVOID arg) {
    SOCKET client_socket = INVALID_SOCKET;
    SA_IN client_addr;
    int addr_size;
    int i = 0;

    TIMEVAL tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100;

    //Main while loop
    printf_s("Waiting for connections...\n");
    while (TRUE) {
        //Wait and accept an incoming connection
        //Addr_size changes with each accept, so a reset is needed
        addr_size = sizeof(SA_IN);
        ready_sockets = current_sockets;

        if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, &tv) < 0) {
            printf_s("Client socket select failed, error code : %d\n", WSAGetLastError());
            ReleaseSemaphore(sinterrupt_main, 1, NULL);
            return 42;
        }

        //Check if there are sockets ready for processing
        for (i = 0; i < ready_sockets.fd_count; i++) {
            if (ready_sockets.fd_array[i] == server_socket) {
                //This is a new connection
                client_socket = accept(server_socket, (SA*)&client_addr, (socklen_t*)&addr_size);

                if (client_socket == INVALID_SOCKET) {
                    printf_s("Client socket accept failed, error code : %d\n", WSAGetLastError());
                    ReleaseSemaphore(sinterrupt_main, 1, NULL);
                    return 1;
                }
                else {
                    printf_s("Connected!\n");
                    FD_SET(client_socket, &current_sockets);
                }
            }
            else
            {
                //Handle the connection
                printf_s("\nHandle the connection!\n");
                SOCKET* pclient = (SOCKET*)malloc(sizeof(SOCKET));
                *pclient = ready_sockets.fd_array[i];

                EnterCriticalSection(&cs);
                FD_CLR(ready_sockets.fd_array[i], &current_sockets);
                enqueue(pclient);
                LeaveCriticalSection(&cs);

                ReleaseSemaphore(semafore, 1, NULL);
            }
        }
    } //while
}

DWORD WINAPI thread_function(LPVOID arg) {
    SOCKET* pclient;
    HANDLE semaphores[2] = { semafore, interrupt_semaphore };

    while (TRUE) {

        EnterCriticalSection(&cs);
        if ((pclient = dequeue()) == NULL) {
            LeaveCriticalSection(&cs);
            if ((WaitForMultipleObjects(2, semaphores, FALSE, INFINITE)) == (WAIT_OBJECT_0 + 1))
                return 0;
        }
        else
        {
            LeaveCriticalSection(&cs);
        }

        if (pclient != NULL) {
            //Conenction found
            if ((handle_connection(pclient)) != 0)
                cleanup(TH_FAIL);
        }
    }

    return 0;
}

int handle_connection(SOCKET *client_socket) {
    int iResult = 0;
    int msgSize = 0;
    char recvbuf[DEFAULT_BUFLEN];  

    SOCKET client_s = *client_socket;
    free(client_socket);
    client_socket = NULL;

    iResult = recv(client_s, recvbuf, sizeof(recvbuf), 0);

    if ((iResult < 0) && (WSAGetLastError() != WSAEWOULDBLOCK)) {
        printf_s("recv failed with error: %d\n", WSAGetLastError());
        
        closesocket(client_s);
        return 1;
    }

    //Parse message
    request req;
    memcpy(&req, recvbuf, iResult);

    
    printf_s("REQUEST: %d\n", req.code);
    switch (req.code)
    {
    case UnregisterService:

        EnterCriticalSection(&cs_hash_table);
        handle_unregister_service(req.data);
        LeaveCriticalSection(&cs_hash_table);

        send_message(client_s, "Odjava uspesna.\n");
        if (closesocket(client_s) == SOCKET_ERROR)
            printf_s("Close socket failed with error : %d\n", WSAGetLastError());

        printf_s("Connection with client closed.\n\n");

        counter++;
        return 0;
    case RegisterService:
        {}
        process p;
        memcpy(&p, req.data, sizeof(process));

        EnterCriticalSection(&cs_hash_table);
        replication_service.register_service(p);
        LeaveCriticalSection(&cs_hash_table);

        send_message(client_s, "Registracija servisa obradjena.\n");

        break;
    case SendData:
        replication_service.send_data(1, req.data, 1);
        send_message(client_s, "Slanje podataka obradjena.\n");

        break;
    case ReceiveData:
        replication_service.receive_data(req.data, 1);
        send_message(client_s, "Primanje podataka obradjena.\n");

        break;
    default:
        send_message(client_s, "Nepoznat zahtev.\n");     
        break;
    }

    EnterCriticalSection(&cs);
    FD_SET(client_s, &current_sockets);
    LeaveCriticalSection(&cs);

    return 0;
}

void handle_unregister_service(char* data) {
    int service_id = -1;
    memcpy(&service_id, data, sizeof(service_id));

    unregister_service(service_id);

    return;
}

SOCKET server_setup() {
    SA_IN server_addr;

    //Server socket creation
    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET) {
        printf_s("Server socket creation failed, error code : %d", WSAGetLastError());
        return server_socket;
    }

    //init the address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(DEFAULT_ADDRESS);
    server_addr.sin_port = htons(server_port);

    //Bind socket
    bind(server_socket, (SA*)&server_addr, sizeof(server_addr));
    if (server_socket == SOCKET_ERROR) {
        printf_s("Server socket binding failed, error code : %d", WSAGetLastError());
        return server_socket;
    }

    //Set nonblocking mode
    unsigned long int nonBlockingMode = 1;
    ioctlsocket(server_socket, FIONBIO, &nonBlockingMode);

    //Set socket to listen mode
    listen(server_socket, SERVER_BACKLOG);
    if (server_socket == SOCKET_ERROR) {
        printf_s("Server socket listen mode failed, error code : %d", WSAGetLastError());
        return server_socket;
    }

    printf_s("Socket listening on port %d\n", server_port);

    return server_socket;
}

#pragma endregion