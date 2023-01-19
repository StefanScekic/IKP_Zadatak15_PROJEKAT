#include "ServerSocket.h"

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType);

int main(int argc, char* argv[]) {    
    //CTRL interupt init
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

    //Check if there are any arguments passed to the process, if not default values will be used
    //replicator [server_port] [replicator_port]
    if (argc > 1) {
        if (argc != 3) {
            printf_s("Wrong number of arguments.\nCorrect format : .\\Replicator.exe [server_port] [replicator_port]\n");
            return -1;
        }

        set_server_port((u_short)(strtoul(argv[1], NULL, 10)));
        set_replicator_port((u_short)(strtoul(argv[2], NULL, 10)));
    }
    
    //Start the server socket
    boot_server_socket();

    //End program interrup signal
    WaitForSingleObject(sinterrupt_main, INFINITE);

    //getchar();
    cleanup(ALL_GOOD);

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