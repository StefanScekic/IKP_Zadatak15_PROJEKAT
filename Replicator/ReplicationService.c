#include "ReplicationService.h"

void register_service(process p) {
	if(hash_table_insert(p))
		printf_s("Insert Successfull.\n");
	else
	{
		printf_s("Insert failed.\n");
	}

	print_table();

	return;
}

void send_data(int service_id, void* data, int data_size) {
	return;
}

void receive_data(void* data, int data_size) {
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