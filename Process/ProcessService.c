#include "ProcessService.h"

void register_service(int service_id) {
    int iResult = 0;    

    request req;
    req.code = RegisterService;
    memcpy(req.data, &service_id, sizeof(service_id));

    iResult = send(get_receive_data_socket(), (char*)&req, sizeof(req.code) + sizeof(service_id), 0);
    if (iResult == SOCKET_ERROR)
    {
        printf_s("Send failed with error code: %d\n", WSAGetLastError());
        return;
    }

	return;
}

void send_data(int service_id, void* data, int data_size) {
    int iResult = 0;

    request req;
    req.code = SendData;
    memcpy(req.data, data, sizeof(file));

    iResult = send(get_send_request_socket(), (char*)&req, sizeof(req.code) + sizeof(file), 0);
    if (iResult == SOCKET_ERROR)
    {
        printf_s("Send failed with error code: %d\n", WSAGetLastError());
        return;
    }

	return;
}

void receive_data(void* data, int data_size) {
    /*file temp_file;
    memcpy(&temp_file, data, data_size);

    if (write_file(temp_file.file_name, temp_file.file_contents, temp_file.file_size) == 0) {
        printf("File replicated successfully\n");
    }*/

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