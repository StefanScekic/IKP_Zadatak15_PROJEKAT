#pragma once

typedef struct ReplicationService {
    void (*registerService)(int ServiceID);
    void (*sendData)(int ServiceID, void* data, int dataSize);
    void (*receiveData)(void* data, int dataSize);
} ReplicationService;

void RegisterService(int ServiceID);
void SendData(int ServiceID, void* data, int dataSize);
void ReceiveData(void* data, int dataSize);