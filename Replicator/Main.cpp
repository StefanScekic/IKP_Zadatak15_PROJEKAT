#include "../Common/Includes.h"
#include "../Common/Connection.h"
#include "../Common/SQueue.h"

#define DEFAULT_BUFLEN 4096
#define THREAD_POOL_SIZE 20

HANDLE threadPool[THREAD_POOL_SIZE];    //thread pool
HANDLE as_thread;
CRITICAL_SECTION cs;
HANDLE semafore, interrupt_semaphore, sinterrupt_main;  //semafori
SOCKET server_socket = INVALID_SOCKET;
fd_set current_sockets, ready_sockets;

DWORD WINAPI handle_connection(LPVOID client_socket);
DWORD WINAPI thread_function(LPVOID arg);
DWORD WINAPI accept_connections_thread_function(LPVOID arg);
SOCKET server_setup(int server_port, int backlog);
void CleanUp();

int counter = 0; //Used for checking if all conections are handled


BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType) {
    if (dwCtrlType == CTRL_C_EVENT) {
        printf("CTRL+C received! Exiting the program...\n");
        ReleaseSemaphore(sinterrupt_main, 1, NULL);
        return TRUE;
    }
    return FALSE;
}

int main() {
    #pragma region Inits
    int i = 0;

    semafore = CreateSemaphore(NULL, 0, 4096, NULL);
    interrupt_semaphore = CreateSemaphore(NULL, 0, THREAD_POOL_SIZE, NULL);
    sinterrupt_main = CreateSemaphore(NULL, 0, 1, NULL);
    InitializeCriticalSection(&cs);

    //Create the thread pool    
    for (i = 0; i < THREAD_POOL_SIZE; i++) {
        threadPool[i] = CreateThread(NULL, 0, thread_function, NULL, 0, 0);
        if (threadPool[i] == NULL) {
            printf("CreateThread failed with error code: %d\n", GetLastError());
            exit(1);
        }
    }


    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
    if (InitializeWindowsSockets() == false)
    {
        return 1;
    }

    FD_ZERO(&current_sockets);
    #pragma endregion
    
    //Setup server
    server_socket = server_setup(SERVERPORT, SERVER_BACKLOG);
    if (server_socket == -1)
        CleanUp();

    FD_SET(server_socket, &current_sockets);

    //Main Accept incoming connections thread
    if ((as_thread = CreateThread(NULL, 0, accept_connections_thread_function, NULL, 0, 0)) == NULL) {
        printf("CreateThread failed with error code: %d\n", GetLastError());
    }
    
    //End program interrup signal
    WaitForSingleObject(sinterrupt_main, INFINITE);

    CleanUp();

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
            return -1;
        }

        //Check if there are sockets ready for processing
        for (i = 0; i < ready_sockets.fd_count; i++) {
            if (ready_sockets.fd_array[i] == server_socket) {
                //This is a new connection
                client_socket = accept(server_socket, (SA*)&client_addr, (socklen_t*)&addr_size);

                if (client_socket == INVALID_SOCKET) {
                    printf("Client socket accept failed, error code : %d\n", WSAGetLastError());
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

SOCKET server_setup(int server_port, int backlog) {
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
    listen(server_socket, backlog);
    if (server_socket == SOCKET_ERROR) {
        printf("Server socket listen mode failed, error code : %d", WSAGetLastError());
        return -1;
    }

    return server_socket;
}

void CleanUp() {
    int i = 0;
    printf("Cleanup started\n");
    closesocket(server_socket);
    if (!ReleaseSemaphore(interrupt_semaphore, THREAD_POOL_SIZE, NULL)) {
        printf("ReleaseSemaphore error: %d\n", GetLastError());
    }
    /*for (i = 0; i < THREAD_POOL_SIZE; i++) {
        ReleaseSemaphore(interrupt_semaphore, 1, NULL);
    }*/

    WaitForMultipleObjects(THREAD_POOL_SIZE, threadPool, TRUE, INFINITE);
    for (i = 0; i < THREAD_POOL_SIZE; i++) {
        CloseHandle(threadPool[i]);
    }

    DeleteCriticalSection(&cs);

    CloseHandle(as_thread);
    CloseHandle(semafore);
    CloseHandle(interrupt_semaphore);

    WSACleanup();

    printf("\n%d\n", counter);
}

DWORD WINAPI handle_connection(LPVOID client_socket) {
    int iResult;
    int msgSize = 0;
    char recvbuf[DEFAULT_BUFLEN];

    SOCKET cs = *((SOCKET*)client_socket);
    free(client_socket);

    while ((iResult = recv(cs, recvbuf + msgSize, sizeof(recvbuf) - msgSize - 1, 0)) > 0) {
        msgSize += iResult;
        if (msgSize > DEFAULT_BUFLEN - 1 || recvbuf[msgSize-1] == '\n') //Izmeniti kad bude trebalo
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
    iResult = send(cs, odgovor, strlen(odgovor), 0);

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
            handle_connection(pclient);
        }
    }

    return 0;
}