#include <stdio.h>
#include "../Common/Connection.h"
#include "../Common/RequestInterface.h"
#include "ClientSocket.h"
#include <time.h>

int main(int argc, char* argv[]) {
    clock_t start_time = clock();

    #pragma region Code
    int iResult = 0;

    if (argc > 1) {
        printf_s("%s\n", argv[1]);
    }

    init_client_sockets(1801);

    send_request(SERVERPORT, RegisterService);
    //getchar();
    send_request(SERVERPORT, RegisterService);

    //Clean-up
    cleanup(ALL_GOOD);
#pragma endregion

    clock_t end_time = clock();
    double execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Execution time: %f seconds\n", execution_time);

    return 0;
}