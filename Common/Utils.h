#pragma once

#ifndef UTILS_H
#define UTILS_H

#include "Includes.h"
#include "Connection.h"

typedef enum ExitCode
{
	ALL_GOOD = 0,	//yay
	WSA_FAIL = -1,	//WindowsSockets fail
	SMF_FAIL = -2,	//Semaphore fail
	TP_FAIL = -3,	//ThreadPool fail
	SC_FAIL = -4,	//Socket fail
	TH_FAIL = -5,	//Thread fail
} exit_code;

const char* get_exit_code_name(exit_code code);

void recieve_message(SOCKET socket);

void send_message(SOCKET cs,const char* answer);

char* read_file(const char* src_path, size_t* file_size);

int write_file(const char* dest_path, char* file_contents, size_t file_size);

#endif // !UTILS_H
