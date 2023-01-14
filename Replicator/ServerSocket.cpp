#include "ServerSocket.h"
#include "SQueue.h"

HANDLE threadPool[THREAD_POOL_SIZE];    //thread pool

CRITICAL_SECTION cs;
HANDLE semafore, interrupt_semaphore, sinterrupt_main;  //semafori
SOCKET server_socket = INVALID_SOCKET;
fd_set current_sockets, ready_sockets;
HANDLE as_thread;

int counter = 0; //Used for checking if all conections are handled
int rollbackCounter = 0; //Counter used for clean up

//Private function to create a server socket
SOCKET server_setup(int server_port) {
    SA_IN server_addr;

    //Server socket creation
    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET) {
        printf("Server socket creation failed, error code : %d", WSAGetLastError());
        return -1;
    }

    //init the address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(DEFAULT_ADDRESS);
    server_addr.sin_port = htons(server_port);

    //Bind socket
    bind(server_socket, (SA*)&server_addr, sizeof(server_addr));
    if (server_socket == SOCKET_ERROR) {
        printf("Server socket binding failed, error code : %d", WSAGetLastError());
        return -1;
    }

    //Set nonblocking mode
    unsigned long int nonBlockingMode = 1;
    ioctlsocket(server_socket, FIONBIO, &nonBlockingMode);

    //Set socket to listen mode
    listen(server_socket, SERVER_BACKLOG);
    if (server_socket == SOCKET_ERROR) {
        printf("Server socket listen mode failed, error code : %d", WSAGetLastError());
        return -1;
    }

    return server_socket;
}

DWORD WINAPI handle_connection(LPVOID client_socket) {
    int iResult;
    int msgSize = 0;
    char recvbuf[DEFAULT_BUFLEN];

    SOCKET cs = *((SOCKET*)client_socket);
    free(client_socket);

    //Recieve msg untill buffer length is exceeded or full msg is recieved
    while ((iResult = recv(cs, recvbuf + msgSize, sizeof(recvbuf) - msgSize - 1, 0)) > 0) {
        msgSize += iResult;
        if (msgSize > DEFAULT_BUFLEN - 1 || recvbuf[msgSize - 1] == '\n') //Izmeniti kad bude trebalo
            break;
    }

    if (iResult < 0) {
        printf("recv failed with error: %d\n", WSAGetLastError());
        closesocket(cs);
        return -1;
    }
    recvbuf[msgSize] = '\0';

    printf("REQUEST: %s\n", recvbuf);

    //Sleep(1000); //Used for thread testing purposes

    const char* odgovor = "odgovor";
    if ((send(cs, odgovor, strlen(odgovor), 0)) == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(cs);
        return -1;
    }    

    if (closesocket(cs) == SOCKET_ERROR)
        printf("Close socket failed with error : %d\n", WSAGetLastError());

    printf("Connection with client closed.\n");
    counter++;

    return 0;
}

DWORD WINAPI thread_function(LPVOID arg) {
    SOCKET* pclient;
    HANDLE semaphores[2] = { semafore, interrupt_semaphore };

    while (true) {

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
            if ((handle_connection(pclient)) < 0)
                cleanup(HC_FAIL);
        }
    }

    return 0;
}

DWORD WINAPI accept_connections_thread_function(LPVOID arg) {
    int i = 0;
    int addr_size;
    SOCKET client_socket = INVALID_SOCKET;
    SA_IN client_addr;

    //Main while loop
    printf("Waiting for connections...\n");
    while (true) {
        //Wait and accept an incoming connection
        //Addr_size changes with each accept, so a reset is needed
        addr_size = sizeof(SA_IN);
        ready_sockets = current_sockets;

        if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0) {
            printf("Client socket select failed, error code : %d\n", WSAGetLastError());
            ReleaseSemaphore(sinterrupt_main, 1, NULL);
            return -1;
        }

        //Check if there are sockets ready for processing
        for (i = 0; i < ready_sockets.fd_count; i++) {
            if (ready_sockets.fd_array[i] == server_socket) {
                //This is a new connection
                client_socket = accept(server_socket, (SA*)&client_addr, (socklen_t*)&addr_size);

                if (client_socket == INVALID_SOCKET) {
                    printf("Client socket accept failed, error code : %d\n", WSAGetLastError());
                    ReleaseSemaphore(sinterrupt_main, 1, NULL);
                    return -2;
                }
                else {
                    printf("Connected!\n");
                    FD_SET(client_socket, &current_sockets);
                }
            }
            else
            {
                //Handle the connection
                printf("Handle the connection!\n");
                SOCKET* pclient = (SOCKET*)malloc(sizeof(SOCKET));
                *pclient = ready_sockets.fd_array[i];

                EnterCriticalSection(&cs);
                enqueue(pclient);
                ReleaseSemaphore(semafore, 1, NULL);
                LeaveCriticalSection(&cs);

                FD_CLR(ready_sockets.fd_array[i], &current_sockets);
            }
        }
    } //while
}

//Private function to initialize ThreadPool
int init_tp() {
    int i = 0;

    //Create the thread pool    
    for (i = 0; i < THREAD_POOL_SIZE; i++) {
        threadPool[i] = CreateThread(NULL, 0, thread_function, NULL, 0, 0);
        if (threadPool[i] == NULL) {
            printf("CreateThread failed with error code: %d\n", GetLastError());
            return -1;
        }
    }

    return 0;
}

void init_resources(int server_port) {
    //WSA init
    if (InitializeWindowsSockets() == false)
        cleanup(WSA_FAIL);

    rollbackCounter = 1;

    //CriticalSection init
    InitializeCriticalSection(&cs);

    rollbackCounter = 2;

    //Semaphore init
    if((semafore = CreateSemaphore(NULL, 0, 4096, NULL)) == NULL)
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

    if ((server_socket = server_setup(server_port)) < 0)
        cleanup(SS_FAIL);

    FD_SET(server_socket, &current_sockets);

    rollbackCounter = 5;
}

void cleanup(int exit_code) {
    int i = 0;
    printf("Cleanup started\n");

    switch (rollbackCounter)
    {
    case 6:
        //Main Serve Thread Cleanup
    case 5:
        //Server Socket cleanup
        closesocket(server_socket);
        CloseHandle(as_thread);
    case 4:
        //ThreadPool cleanup
        if (!ReleaseSemaphore(interrupt_semaphore, THREAD_POOL_SIZE, NULL)) {
            printf("ReleaseSemaphore error: %d\n", GetLastError());
        }

        WaitForMultipleObjects(THREAD_POOL_SIZE, threadPool, TRUE, INFINITE);
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
    case 1:
        //WSA cleanup
        WSACleanup();
    case 0:
        if (exit_code != ALL_GOOD) {
            printf("\nExiting ServerAcceptSocket with exit code : %d\n", exit_code);
            exit(exit_code);
        }
    }    

    printf("\n%d\n", counter);
    return;
}

void boot_server_socket(int server_port) {
    init_resources(server_port);

    //Main Accept incoming connections thread
    if ((as_thread = CreateThread(NULL, 0, accept_connections_thread_function, NULL, 0, 0)) == NULL) {
        printf("CreateThread failed with error code: %d\n", GetLastError());
        cleanup(MT_FAIL);
    }
    else
    {
        rollbackCounter = 6;
    }

    return;
}