#include <stdio.h>
#include "../Common/Connection.h"
#include "ClientSocket.h"

int main(int argc, char* argv[]) {
    int iResult = 0;

    if (argc > 1) {
        printf_s("%s\n", argv[1]);
    }

    init_client_sockets(1801);

    iResult = send_request(SERVERPORT);
    handle_send_request_result((SEND_REQUEST_RESULT)iResult);

    //Clean-up
    getchar();
    cleanup(ALL_GOOD);

    return 0;
}