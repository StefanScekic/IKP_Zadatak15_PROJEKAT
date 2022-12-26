#include <stdio.h>
#include "Interface.h"

ReplicationService initRepl();

int main()
{
    int dataSize = 5;
    void* data = &dataSize;

    ReplicationService service = initRepl();

    service.receiveData(data, dataSize);

    return 0;
}

ReplicationService initRepl() {
    ReplicationService service;

    service.receiveData = ReceiveData;
    service.registerService = RegisterService;
    service.sendData = SendData;

    return service;
}