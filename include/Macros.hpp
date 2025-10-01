#pragma once

#include <iostream>
#include <stdlib.h>

using namespace std;

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define ASSERT(condition)                                                                                          \
	if (!(condition)) {                                                                                            \
		cerr << "Assertion failed: " << #condition << ", file " << __FILE__ << ", line " << __LINE__ << std::endl; \
		exit(EXIT_FAILURE);                                                                                        \
	}
