#pragma once
#include "../Common/Includes.h"
#include "../Common/RequestInterface.h"
#include "HashTable.h"
#include "ServerSocket.h"

#ifndef REPLICATION_SERVICE_H
#define REPLICATION_SERVICE_H

extern service_interface replication_service;

void unregister_service(int id);

#endif // !REPLICATION_SERVICE_H