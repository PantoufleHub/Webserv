#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <arpa/inet.h>

using namespace std;

const string LOG_FOLDER = "misc/logs/";
const string CONN_LOG_FILE = LOG_FOLDER + "connections.log";
const string REQ_LOG_FILE = LOG_FOLDER + "requests.log";
const string RESP_LOG_FILE = LOG_FOLDER + "responses.log";
const string ERROR_LOG_FILE = LOG_FOLDER + "errors.log";

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
