#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <WS2tcpip.h>
#include <windows.h>
//Dodaje dependancy u podesavanje svakog projekta koji ovo includuje
#pragma comment(lib, "Ws2_32.lib")