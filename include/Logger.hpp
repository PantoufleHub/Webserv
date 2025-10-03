#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <arpa/inet.h>
#include "Constants.hpp"

using namespace std;

class Logger {
   private:
	static string _getNow();
	static void _log(ofstream& log, const ostringstream& os);
	
   public:
	static void logError(const std::string& message);
	static void logResponse(const string& response, int socket_fd);
	static void logRequest(const string& request, int socket_fd);
	static void logConnection(const sockaddr_in& addr, int socket_fd);
	static void logDisconnection(int socket_fd);
};
