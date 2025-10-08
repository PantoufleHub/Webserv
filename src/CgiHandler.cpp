#include "CgiHandler.hpp"

// TEMP HEADERS
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>

void CgiHandler::_init_() {
	_state = CGI_PROCESSING;
	_error_code = 0;
}

// waaaa mais qui me laisse coder
CgiHandler::CgiHandler(	const HttpResponse	&response,
						const HttpRequest	&request,
						const WebServer		&web_server,
						const VirtualServer	&client_server,
						const Socket 		&client_socket) : 
						_response(response),
						_request(request),
						_web_server(web_server),
						_client_server(client_server),
						_client_socket(client_socket) {
	_init_();

	parseInfo();

	if (_error_code != 0) {
		_state = CGI_ERROR;
		return;
	}

	if (pipe(_pipe) == -1) {
		cout << "Pipe error" << endl;
		_state = CGI_ERROR;
		return;
	}

	_child_pid = fork();
	if (_child_pid == 0) {
		// extern char **environ;
		// string path = (string("www/cgi-bin") + _request->getPath());
		// char* const argv[] = { const_cast<char*>(path.c_str()), NULL };
		// cout << "execveing path: " << path << endl;
		// execve(path.c_str(), argv, environ);
		sleep(5);

		exit(0);
	} else {

	}
}

void CgiHandler::parseInfo() {
	// Get required info and put it in environ
	(void)_response;
	(void)_web_server;
	(void)_client_server;
	(void)_client_socket;
	(void)_request;
	// AUTH_TYPE
    // CONTENT_LENGTH
    // CONTENT_TYPE
    // GATEWAY_INTERFACE.
    // PATH_INFO
    // PATH_TRANSLATED
    // QUERY_STRING
    // REMOTE_ADDR
    // REMOTE_HOST
    // REMOTE_IDENT
    // REMOTE_USER
    // REQUEST_METHOD
    // SCRIPT_NAME
    // SERVER_NAME
    // SERVER_PORT
    // SERVER_PROTOCOL
    // SERVER_SOFTWARE
}

void CgiHandler::update() {
	if (_state == CGI_ERROR)
		return;

	int result;

	result = waitpid(_child_pid, &_child_status, WNOHANG);
	if (result == 0) {
		// parent behaviour (Will be parsing response i think)
	} else if (result == _child_pid) {
		cout << "Cgi finished" << endl;
		_state = CGI_FINISHED;
	}
}

const CgiState &CgiHandler::getState() const {
	return _state;
}

const int &CgiHandler::getErrorCode() const {
	return _error_code;
}
