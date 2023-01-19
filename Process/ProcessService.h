#pragma once
#include "../Common/Includes.h"
#include "../Common/RequestInterface.h"
#include "ClientSocket.h"

#ifndef PROCESS_SERVICE_H
#define PROCESS_SERVICE_H

extern service_interface process_service;

void unregister_service(int id);

#endif // !PROCESS_SERVICE_H
