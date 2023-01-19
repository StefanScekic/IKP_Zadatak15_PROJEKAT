#include "Utils.h"

const char* get_exit_code_name(exit_code code) {
	switch (code)
	{
	case ALL_GOOD:
		return "ALL_GOOD";
	case WSA_FAIL:
		return "WSA_FAIL";
	case SMF_FAIL:
		return "SMF_FAIL";
	case TP_FAIL:
		return "TP_FAIL";
	case SC_FAIL:
		return "SC_FAIL";
	case TH_FAIL:
		return "TH_FAIL";
	default:
		return "UNKNOWS_EXIT_CODE";
	}
}

void recieve_message(SOCKET socket) {
	int iResult = 0;
	char buffer[DEFAULT_BUFLEN];
	//Recieve msg untill buffer length is exceeded or full msg is recieved
	while ((iResult = recv(socket, buffer, sizeof(buffer), 0)) > 0) {
		if (buffer[iResult - 1] == '\n') //Izmeniti kad bude trebalo
			break;
	}

	if ((iResult < 0) && (WSAGetLastError() != WSAEWOULDBLOCK)) {
		printf_s("Receive failed with error code: %d\n", WSAGetLastError());
		return;
	}
	buffer[iResult] = '\0';

	printf_s("RESPONSE: %s\n", buffer);
}

void send_message(SOCKET cs,const char* answer) {
	if ((send(cs, answer, strlen(answer), 0)) == SOCKET_ERROR) {
		printf_s("Send failed with error: %d\n", WSAGetLastError());
	}
}