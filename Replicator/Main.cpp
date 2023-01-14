#include "ServerSocket.h"

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType);

int main() {
    //CTRL interupt init
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

    //Start the server socket
    boot_server_socket(SERVERPORT);

    //End program interrup signal
    WaitForSingleObject(sinterrupt_main, INFINITE);

    cleanup(ALL_GOOD);

    return 0;
}

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType) {
    if (dwCtrlType == CTRL_C_EVENT) {
        printf("CTRL+C received! Exiting the program...\n");
        ReleaseSemaphore(sinterrupt_main, 1, NULL);
        return TRUE;
    }
    return FALSE;
}