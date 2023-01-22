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

int write_file(const char* dest_path, char* file_contents, size_t file_size) {
	FILE* dest_file = fopen(dest_path, "wb"); // open the file in binary mode
	if (dest_file == NULL) {
		printf("Error opening file\n");
		return -1;
	}

	// Write the contents of the buffer to the file
	size_t bytes_written = fwrite(file_contents, sizeof(char), file_size, dest_file);
	if (bytes_written != file_size) {
		printf("Error writing file\n");
		fclose(dest_file);
		return -1;
	}

	fclose(dest_file);
	return 0;
}

char* read_file(const char* src_path, size_t* file_size) {
	FILE* src_file = fopen(src_path, "rb"); // open the file in binary mode
	if (src_file == NULL) {
		printf("Error opening file\n");
		return NULL;
	}

	// Get the size of the file
	fseek(src_file, 0, SEEK_END);
	*file_size = ftell(src_file);
	rewind(src_file);

	// Allocate memory for the contents of the file
	char* buffer = (char*)malloc(*file_size);
	if (buffer == NULL) {
		printf("Error allocating memory\n");
		fclose(src_file);
		return NULL;
	}

	// Read the contents of the file into the buffer
	size_t bytes_read = fread(buffer, sizeof(char), *file_size, src_file);
	if (bytes_read != *file_size) {
		printf("Error reading file\n");
		free(buffer);
		fclose(src_file);
		return NULL;
	}

	fclose(src_file);
	return buffer;
}