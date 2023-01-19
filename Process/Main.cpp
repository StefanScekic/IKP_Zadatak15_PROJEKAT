#include <stdio.h>
#include "../Common/Connection.h"
#include "../Common/RequestInterface.h"
#include "ClientSocket.h"
#include <time.h>

int main(int argc, char* argv[]) {
    clock_t start_time = clock();
    u_short client_port = 1801;
    u_short server_port = SERVERPORT;

    #pragma region Code
    int iResult = 0;

    //Check if there are any arguments passed to the process, if not default values will be used
    //process [client_id] [client_port] [server_port]
    if (argc > 1) {
        if (argc != 4) {
            printf_s("Wrong number of arguments.\nCorrect format : .\\Process.exe [client_id] [client_port] [server_port]\n");
            return -1;
        }

        set_process_id(strtol(argv[1], NULL, 10));
        client_port = (u_short)(strtoul(argv[2], NULL, 10));
        server_port = (u_short)(strtoul(argv[3], NULL, 10));
    }

    //Start the sockets
    init_client_sockets(&client_port, &server_port);

    //Do something
    send_request(SERVERPORT, RegisterService);
    send_request(SERVERPORT, RegisterService);

    //Clean-up
    getchar();
    cleanup(ALL_GOOD);
#pragma endregion

    clock_t end_time = clock();
    double execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Execution time: %f seconds\n", execution_time);

    return 0;
}