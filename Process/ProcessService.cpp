#include "ProcessService.h"

int listen_socket_id = 0;

void recieve_message() {
    char buffer[DEFAULT_BUFLEN];
    int iResult = 0;
    //Recieve msg untill buffer length is exceeded or full msg is recieved
    while ((iResult = recv(send_request_socket, buffer, sizeof(buffer), 0)) > 0) {
        if (buffer[iResult - 1] == '\n') //Izmeniti kad bude trebalo
            break;
    }

    if ((iResult < 0) && (WSAGetLastError() != WSAEWOULDBLOCK)) {
        handle_send_request_result(NO_RESPONSE);
        return;
    }
    buffer[iResult] = '\0';

    printf_s("RESPONSE: %s\n", buffer);
}

void register_service(process p) {
    char buffer[DEFAULT_BUFLEN];
    int iResult = 0;       


    request req;
    req.code = RegisterService;
    memcpy(req.data, &p, sizeof(p));

    iResult = send(send_request_socket, (char*)&req, sizeof(req.code) + sizeof(process), 0);
    if (iResult == SOCKET_ERROR)
    {
        handle_send_request_result(SEND_FAIL);
        return;
    }

    printf_s("Bytes Sent: %ld\n", iResult);

    recieve_message();

    listen_socket_id = p.ID;

	return;
}

void send_data(int service_id, void* data, int data_size) {
	return;
}

void receive_data(void* data, int data_size) {
	return;
}

service_interface process_service = {
	register_service,
	send_data,
	receive_data
};

void unregister_service() {
    int iResult = 0;

    request req;
    req.code = UnregisterService;
    memcpy(req.data, &listen_socket_id, sizeof(listen_socket_id));

    iResult = send(send_request_socket, (char*)&req, sizeof(req.code) + sizeof(listen_socket_id), 0);
    if (iResult == SOCKET_ERROR)
    {
        handle_send_request_result(SEND_FAIL);
        return;
    }

    recieve_message();

    return;
}