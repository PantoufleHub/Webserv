#include "Logger.hpp"

using namespace std;

static ofstream _connection_log(CONN_LOG_FILE.c_str(), ios::trunc);
static ofstream _request_log(REQ_LOG_FILE.c_str(), ios::trunc);
static ofstream& _disconnection_log = _connection_log;  // Reuse connection log for disconnections
static ofstream _response_log(RESP_LOG_FILE.c_str(), ios::trunc);
static ofstream _error_log(ERROR_LOG_FILE.c_str(), ios::trunc);

void Logger::_log(ofstream& log, const ostringstream& os) {
	if (!log.is_open())
		cout << "Log not open" << std::endl;
	else {
		log << "| " << _getNow() << " | " << os.str() << endl;
	}
}

/// @brief Logs a HTTP response sent
/// @param response The response sent
/// @param socket_fd The socket the response was sent to
void Logger::logResponse(const string& response, int socket_fd) {
	ostringstream response_oss;

	response_oss << "Towards socket " << socket_fd << "\n" << response << endl;

	_log(_response_log, response_oss);
}

/// @brief Logs a HTTP request received
/// @param request The request received
/// @param socket_fd The socket it was received by
void Logger::logRequest(const string& request, int socket_fd) {
	ostringstream request_oss;

	request_oss << "From socket " << socket_fd << "\n" << request << endl;

	_log(_request_log, request_oss);
}

/// @brief Logs a new client connection
/// @param addr The address info of the new client
/// @param socket_fd The new socket fd created for the client
void Logger::logConnection(const sockaddr_in& addr, int socket_fd) {
	ostringstream connection_oss;

	connection_oss << "New connection\n"
	               << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << " => socket " << socket_fd << endl;

	_log(_connection_log, connection_oss);
}

/// @brief Logs a client disconnection
/// @param socket_fd The client socket
void Logger::logDisconnection(int socket_fd) {
	ostringstream disconnect_oss;

	disconnect_oss << "Disconnection\nSocket " << socket_fd << endl;

	_log(_disconnection_log, disconnect_oss);
}

/// @brief Logs an error
/// @param error The error duh
// void Logger::logError(const string& error) {
// 	ostringstream error_oss;

// 	error_oss << error << endl;

// 	_log(_error_log, error_oss);
// }

void Logger::logError(const std::string& message) {
	std::cout << "[Error] " << message << std::endl;
}

/// @brief Get current time
/// @return A string of current date (yyyy-mm-dd HH:MM:SS)
string Logger::_getNow() {
	string ret;
	time_t t = time(NULL);
	tm* now = localtime(&t);
	char buffer[80];

	strftime(buffer, 80, "%x %T", now);
	ostringstream ss;
	ss << buffer;

	ret = ss.str();

	return ret;
}
