#pragma once
#include "../Common/Includes.h"
#include "../Common/RequestInterface.h"
#include "ClientSocket.h"

#ifndef PROCESS_SERVICE_H
#define PROCESS_SERVICE_H

extern service_interface process_service;
extern int listen_socket_id;

void unregister_service();

#endif // !PROCESS_SERVICE_H
