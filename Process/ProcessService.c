#include "ProcessService.h"

void register_service(process p) {
    int iResult = 0;    

    request req;
    req.code = RegisterService;
    memcpy(req.data, &p, sizeof(p));

    iResult = send(get_send_request_socket(), (char*)&req, sizeof(req.code) + sizeof(process), 0);
    if (iResult == SOCKET_ERROR)
    {
        printf_s("Send failed with error code: %d\n", WSAGetLastError());
        return;
    }

    //printf_s("Bytes Sent: %ld\n", iResult);

    recieve_message(get_send_request_socket());
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

void unregister_service(int id) {
    int iResult = 0;

    request req;
    req.code = UnregisterService;
    memcpy(req.data, &id, sizeof(id));

    iResult = send(get_send_request_socket(), (char*)&req, sizeof(req.code) + sizeof(id), 0);
    if (iResult == SOCKET_ERROR)
    {
        printf_s("Send failed with error code: %d\n", WSAGetLastError());
        return;
    }

    recieve_message(get_send_request_socket());

    return;
}