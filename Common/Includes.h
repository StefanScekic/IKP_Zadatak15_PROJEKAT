#pragma once

#define FD_SETSIZE 1024
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <WinSock2.h>
//Dodaje dependancy u podesavanje svakog projekta koji ovo includuje
#pragma comment(lib, "Ws2_32.lib")