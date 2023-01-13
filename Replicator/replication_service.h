#ifndef REPLICATION_SERVICE_H
#define REPLICATION_SERVICE_H


void RegisterServic(int ServiceID);
void SendDat(int ServiceID, void* data, int dataSize);
void ReceiveData(void* data, int dataSize);


#endif // REPLICATION_SERVICE_H