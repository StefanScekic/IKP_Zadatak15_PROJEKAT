#include <stdio.h>
#include "ClientSocket.h"
#include <time.h>

int main(int argc, char* argv[]) {
    clock_t start_time = clock();

    #pragma region Code
    //Check if there are any arguments passed to the process, if not default values will be used
    //process [client_id] [client_port] [server_port]
    if (argc > 1) {
        if (argc != 4) {
            printf_s("Wrong number of arguments.\nCorrect format : .\\Process.exe [client_id] [client_port] [server_port]\n");
            return -1;
        }

        set_process_id(strtol(argv[1], NULL, 10));
        set_client_port((u_short)(strtoul(argv[2], NULL, 10)));
        set_server_port((u_short)(strtoul(argv[3], NULL, 10)));
    }

    //Start the sockets
    init_client_sockets();

    //Do something
    send_request(RegisterService);
    send_request(RegisterService);

    //Clean-up
    //getchar();
    cleanup(ALL_GOOD);
#pragma endregion

    clock_t end_time = clock();
    double execution_time = (double)((end_time - start_time) / CLOCKS_PER_SEC);
    printf("Execution time: %f seconds\n", execution_time);

    return 0;
}