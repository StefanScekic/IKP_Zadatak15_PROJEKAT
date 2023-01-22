#include "ServerSocket.h"

HANDLE threadPool[THREAD_POOL_SIZE];        //Thread pool

SOCKET server_socket = INVALID_SOCKET;      //Accept requests socket
SOCKET rtr_socket = INVALID_SOCKET;         //Send data to be replicated socket
CRITICAL_SECTION cs;                        //Enqueue/Dequeue critical section
CRITICAL_SECTION cs_hash_table;             //HashTable critical section
fd_set current_sockets;                     //FDs of socekts select is listening on
fd_set ready_sockets;                       //Helper FD set, select is destructive

HANDLE sinterrupt_main = NULL;              //End program semaphore
HANDLE semafore = NULL;                     //There is work semaphore
HANDLE interrupt_semaphore = NULL;          //Close worker threads semaphore
HANDLE as_thread = NULL;                    //Accept requests socket thread
HANDLE repl_repl_thread = NULL;             //Replicator to replicator connection thread

int counter = 0;                            //Counts the number of connections handled
int rollbackCounter = 0;                    //Counter used for clean up
BOOL shutdows_signal = FALSE;                //cheat

#pragma region ServerPort
u_short server_port = DEFAULT_SERVER_PORT;

void set_server_port(u_short port) {
    server_port = port;
}

u_short get_server_port() {
    return server_port;
}
#pragma endregion

#pragma region ReplicatorPort
u_short replicator_port = DEFAULT_REPLICATOR_CONNECT_PORT;       //Port for server to conect to another replicator

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
    Function ran by thread that communicates with another Replicator
*/
DWORD WINAPI rtr_thread(LPVOID arg);

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
    Make a socket and put it in listen mode

    @param port: port it will listen on
    @param mode: 0 for non blocking, 1 for blocking
*/
SOCKET setup_socket(u_short port, u_long mode);

#pragma endregion

#pragma region PublicFunctions

SOCKET get_rtr_socket() {
    return rtr_socket;
}

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
        WaitForSingleObject(as_thread, INFINITE);
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

    server_socket = setup_socket(server_port, 1);
    if ((server_socket == INVALID_SOCKET) || (server_socket == SOCKET_ERROR))
        cleanup(SC_FAIL);

    FD_SET(server_socket, &current_sockets);

    if (replicator_port != DEFAULT_REPLICATOR_CONNECT_PORT) {   //If port was not changed, run app in test mode
        rtr_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if ((rtr_socket == INVALID_SOCKET))
            cleanup(SC_FAIL);
    }
    rollbackCounter = 5;

    //HashTable init
    init_hash_table();

    rollbackCounter = 6;

    //Main Accept incoming connections thread
    if ((as_thread = CreateThread(NULL, 0, accept_connections_thread_function, NULL, 0, 0)) == NULL) {
        printf_s("CreateThread failed with error code: %d\n", GetLastError());
        cleanup(TH_FAIL);
    }

    //Replicator - Replicator communication thread
    if (rtr_socket != INVALID_SOCKET) {
        if ((as_thread = CreateThread(NULL, 0, rtr_thread, NULL, 0, 0)) == NULL) {
            printf_s("CreateThread failed with error code: %d\n", GetLastError());
            cleanup(TH_FAIL);
        }
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

    if (rtr_socket != INVALID_SOCKET)
        closesocket(rtr_socket);
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

DWORD WINAPI rtr_thread(LPVOID arg) {
    int iResult = 0;

    SA_IN server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(DEFAULT_ADDRESS);
    server_addr.sin_port = htons(replicator_port);

    while(TRUE)
    {
        if (shutdows_signal)
            return 22;

        iResult = connect(rtr_socket, (SA*)&server_addr, sizeof(server_addr));
        if (iResult != SOCKET_ERROR) {
            printf_s("Connection with another replicator established!\n");
            break;
        }

        printf_s("Connect failed with error code: %d\nTrying again in 2s.\n", WSAGetLastError());
        Sleep(2000);
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
    printf_s("Replicator request socket waiting for connections...\n");
    while (TRUE) {
        //Wait and accept an incoming connection
        //Addr_size changes with each accept, so a reset is needed
        addr_size = sizeof(SA_IN);
        ready_sockets = current_sockets;

        if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, &tv) < 0) {
            printf_s("Client socket select failed, error code : %d\n", WSAGetLastError());
            shutdows_signal = TRUE;
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
            /*if ((handle_connection(pclient)) != 0)
                cleanup(TH_FAIL);*/
            handle_connection(pclient);
        }
    }

    return 0;
}

int handle_connection(SOCKET *client_socket) {
    int iResult = 0;
    int msgSize = 0;
    char recvbuf[sizeof(request)];

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
        memcpy(&p.ID, req.data, sizeof(int));
        p.socket = client_s;

        EnterCriticalSection(&cs_hash_table);
        if (hash_table_insert(p))
            printf_s("Insert Successfull.\n");
        else
        {
            printf_s("Insert failed.\n");
        }

        print_table();
        LeaveCriticalSection(&cs_hash_table);

        break;
    case SendData:
        {}
        file temp_file;
        memcpy(&temp_file, req.data, sizeof(temp_file));

        replication_service.send_data(temp_file.ownder_id, req.data, sizeof(req.data));

        process send_data_p;
        if (hash_table_lookup(temp_file.ownder_id, &send_data_p)) {
            send_message(send_data_p.socket, "Slanje podataka obradjeno.\n");
        }

        break;
    case Callback:
        {}
        process p2;
        file temp_file_callback;
        memcpy(&temp_file_callback, req.data, sizeof(temp_file_callback));

        if (hash_table_lookup(temp_file_callback.ownder_id, &p2)) {
            replication_service.receive_data(&temp_file_callback, sizeof(file));
            send_message(client_s, "Podatci za replikaciju primljeni.\n");
        }
        else
        {
            send_message(client_s, "Proces koji treba da primi podatke za replikaciju nije aktivan.\n");
        }

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

SOCKET setup_socket(u_short port, u_long mode) {
    SOCKET return_socket = INVALID_SOCKET;
    SA_IN server_addr;

    //Server socket creation
    return_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (return_socket == INVALID_SOCKET) {
        printf_s("Server socket creation failed, error code : %d", WSAGetLastError());
        return return_socket;
    }

    //init the address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(DEFAULT_ADDRESS);
    server_addr.sin_port = htons(port);

    //Bind socket
    bind(return_socket, (SA*)&server_addr, sizeof(server_addr));
    if (return_socket == SOCKET_ERROR) {
        printf_s("Server socket binding failed, error code : %d", WSAGetLastError());
        return return_socket;
    }

    //Set nonblocking mode
    u_long nonBlockingMode = mode;
    ioctlsocket(return_socket, FIONBIO, &nonBlockingMode);

    //Set socket to listen mode
    listen(return_socket, SERVER_BACKLOG);
    if (return_socket == SOCKET_ERROR) {
        printf_s("Server socket listen mode failed, error code : %d", WSAGetLastError());
        return return_socket;
    }

    printf_s("Socket listening on port %d\n", port);

    return return_socket;
}

#pragma endregion