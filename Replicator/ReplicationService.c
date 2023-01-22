#include "ReplicationService.h"

void register_service(int service_id) {
	

	return;
}

void send_data(int service_id, void* data, int data_size) {
	int iResult = 0;

	request req;
	req.code = Callback;
	memcpy(req.data, data, data_size);

	iResult = send(get_rtr_socket(), (char*)&req, sizeof(req), 0);
	if (iResult == SOCKET_ERROR)
	{
		printf_s("Send failed with error code: %d\n", WSAGetLastError());
		return;
	}

	recieve_message(get_rtr_socket());

	return;
}

void receive_data(void* data, int data_size) {
	int iResult = 0;

	file temp_file;
	memcpy(&temp_file, data, data_size);

	process p;
	hash_table_lookup(temp_file.ownder_id, &p);

	iResult = send(p.socket, (char*)&temp_file, sizeof(temp_file), 0);
	if (iResult == SOCKET_ERROR)
	{
		printf_s("Send failed with error code: %d\n", WSAGetLastError());
		return;
	}

	return;
}

service_interface replication_service = {
	register_service,
	send_data,
	receive_data
};

void unregister_service(int id) {
	if(hash_table_delete(id))
		printf_s("Delete Successfull\n");
	else {
		printf_s("Delete Failed\n");
	}

	print_table();

	return;
}