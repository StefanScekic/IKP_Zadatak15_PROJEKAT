#pragma once

#ifndef PROCESS_SERVICE_H
#define PROCESS_SERVICE_H

#include "ClientSocket.h"
#include "../Common/Includes.h"
#include "../Common/RequestInterface.h"
#include "../Common/Utils.h"

extern service_interface process_service;

void unregister_service(int id);

#endif // !PROCESS_SERVICE_H