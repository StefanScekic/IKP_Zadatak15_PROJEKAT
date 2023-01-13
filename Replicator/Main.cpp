#include "../Common/Includes.h"
#include "../Common/Connection.h"
#include "../Common/SQueue.h"

#define DEFAULT_BUFLEN 512
#define THREAD_POOL_SIZE 20

HANDLE threadPool[THREAD_POOL_SIZE];
CRITICAL_SECTION cs;
HANDLE semafore;

DWORD WINAPI handle_connection(LPVOID client_socket);
DWORD WINAPI thread_function(LPVOID arg);

int counter = 0;
int condition = 0;

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType) {
    if (dwCtrlType == CTRL_C_EVENT) {
        printf("CTRL+C received! Exiting the loop...\n");
        condition = 1;
        return TRUE;
    }
    return FALSE;
}

int main() {
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
    int i = 0;

    if (InitializeWindowsSockets() == false)
    {
        return 1;
    }

    InitializeCriticalSection(&cs);
    semafore = CreateSemaphore(0, 0, 4096, NULL);

    fd_set current_sockets, ready_sockets;
    FD_ZERO(&current_sockets);

    //Sockets init
    SOCKET server_socket = INVALID_SOCKET;
    SOCKET client_socket = INVALID_SOCKET;
    SA_IN server_addr, client_addr;
    int addr_size;
    
    //Server socket creation
    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET) {
        printf("Server socket creation failed, error code : %d", WSAGetLastError());

        WSACleanup();
        return 1; 
    }    


    //init the address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(DEFAULT_ADDRESS);
    server_addr.sin_port = htons(SERVERPORT);

    //Bind socket
    bind(server_socket, (SA*)&server_addr, sizeof(server_addr));
    if (server_socket == SOCKET_ERROR) {
        printf("Server socket binding failed, error code : %d", WSAGetLastError());

        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    //Set socket to listen mode
    listen(server_socket, SERVER_BACKLOG);
    if (server_socket == SOCKET_ERROR) {
        printf("Server socket listen mode failed, error code : %d", WSAGetLastError());

        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    unsigned long int nonBlockingMode = 1;
    ioctlsocket(server_socket, FIONBIO, &nonBlockingMode);

    FD_SET(server_socket, &current_sockets);

    //Create the thread pool
    
    for (i = 0; i < THREAD_POOL_SIZE; i++) {
        threadPool[i] = CreateThread(NULL, 0, thread_function, NULL, 0, 0);
    }

    while (true) {
        ready_sockets = current_sockets;
        printf("Waiting for connections...\n");

        //Wait and accept an incoming connection
        //Addr_size changes with each accept, so a reset is needed
        addr_size = sizeof(SA_IN);

        if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0) {
            printf("Client socket accept failed, error code : %d", WSAGetLastError());
            break;
        }

        for (i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &ready_sockets)) {
                if (i == server_socket) {
                    //this is a new connection
                    client_socket = accept(server_socket, (SA*)&client_addr, (socklen_t*)&addr_size);
                    if (client_socket == INVALID_SOCKET) {
                        printf("Client socket accept failed, error code : %d", WSAGetLastError());

                        break;
                    }
                    else {
                        printf("Connected!\n");
                        FD_SET(client_socket, &current_sockets);
                    }
                }
                else
                {
                    //Handle the connection
                    SOCKET *pclient = (SOCKET*)malloc(sizeof(SOCKET));
                    *pclient = i;

                    EnterCriticalSection(&cs);
                    enqueue(pclient);
                    ReleaseSemaphore(semafore, 1, NULL);
                    LeaveCriticalSection(&cs);

                    FD_CLR(i, &current_sockets);
                }
            }
        }

        if (condition) {
            break;
        }
    } //while

    //Clean-up
    for (i = 0; i < THREAD_POOL_SIZE; i++) {
        CloseHandle(threadPool[i]);
    }

    DeleteCriticalSection(&cs);
    closesocket(server_socket);
    CloseHandle(semafore);
    WSACleanup();

    printf("\n%d\n", counter);

    return 0;
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
    //_flushall();

    //Sleep(1000); //Used for thread testing purposes

    const char* odgovor = "odgovor";
    iResult = send(cs, odgovor, strlen(odgovor), 0);

    if (closesocket(cs) == SOCKET_ERROR)
        printf("Close socket failed with error : %d", WSAGetLastError());

    printf("Connection with client closed.\n");
    counter++;
    _flushall();

    return 0;
}

DWORD WINAPI thread_function(LPVOID arg) {
    while (true) {
        SOCKET* pclient;

        EnterCriticalSection(&cs);
        if ((pclient = dequeue()) == NULL) {
            LeaveCriticalSection(&cs);
            WaitForSingleObject(semafore, INFINITE);
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