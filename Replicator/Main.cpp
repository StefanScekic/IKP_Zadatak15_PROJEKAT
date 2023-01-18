#include "ServerSocket.h"

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType);

int main() {
    u_int port = SERVERPORT;
    //CTRL interupt init
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

    //printf_s("Server starting up...\nChoose a port : ");
    //scanf_s("%d", &port);   //I'm not protectiong this from invalid inputs
    
    //Start the server socket
    boot_server_socket(port);

    //End program interrup signal
    WaitForSingleObject(sinterrupt_main, INFINITE);

    cleanup(ALL_GOOD);
    //getchar();

    return 0;
}

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType) {
    if (dwCtrlType == CTRL_C_EVENT) {
        printf_s("CTRL+C received! Exiting the program...\n");
        ReleaseSemaphore(sinterrupt_main, 1, NULL);
        return TRUE;
    }
    return FALSE;
}