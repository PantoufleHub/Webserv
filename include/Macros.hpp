#pragma once

#include <iostream>
#include <stdlib.h>

using namespace std;

#define SERVER_SOFTWARE "PLAP/1.0"

#define DEFAULT_SOCKET_BACKLOG 1024
#define DEFAULT_BUFFER_SIZE 10240
#define POLL_TIMEOUT 0

#define CLIENT_TIMEOUT 10
#define CGI_TIMEOUT 5

#define PYTHON_EXTENSION ".py"
#define SHELL_EXTENSION ".sh"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define READ 0
#define WRITE 1

#define ASSERT(condition)                                                                                          \
	if (!(condition)) {                                                                                            \
		cout << "Assertion failed: " << #condition << ", file " << __FILE__ << ", line " << __LINE__ << std::endl; \
		exit(EXIT_FAILURE);                                                                                        \
	}
