#pragma once

#include <iostream>
#include <stdlib.h>

using namespace std;

#define DEFAULT_SOCKET_BACKLOG 1024
#define DEFAULT_BUFFER_SIZE 10

#define PYTHON_EXTENSION ".py"
#define SHELL_EXTENSION ".sh"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define ASSERT(condition)                                                                                          \
	if (!(condition)) {                                                                                            \
		cerr << "Assertion failed: " << #condition << ", file " << __FILE__ << ", line " << __LINE__ << std::endl; \
		exit(EXIT_FAILURE);                                                                                        \
	}
