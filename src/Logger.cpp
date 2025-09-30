#include "Logger.hpp"

#include <iostream>

void Logger::logError(const std::string& message) {
	std::cerr << "[config] " << message << std::endl;
}
