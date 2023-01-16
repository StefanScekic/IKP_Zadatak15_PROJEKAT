#include "ReplicationService.h"

void register_service(int service_id) {
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