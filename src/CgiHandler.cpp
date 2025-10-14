#include "CgiHandler.hpp"

// TEMP HEADERS
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>

void CgiHandler::_init_() {
	_state = CGI_PROCESSING;
	_error_code = 0;
}

CgiHandler::CgiHandler(	const HttpResponse	&response,
						const HttpRequest	&request,
						const VirtualServer	&client_server,
						const Socket 		&client_socket) : 
						_response(response),
						_request(request),
						_client_server(client_server),
						_client_socket(client_socket) {
	_init_();

	_parseInfo();
	if (_error_code != 0) {
		cout << "CGI parsing error" << endl;
		_changeState(CGI_ERROR, HTTP_CODE_INTERNAL_SERVER_ERROR);
		return;
	}

	if (pipe(_pipe) == -1) {
		cout << "Pipe error" << endl;
		_changeState(CGI_ERROR, HTTP_CODE_INTERNAL_SERVER_ERROR);
		return;
	}

	_child_pid = fork();
	if (_child_pid == 0) {
		extern char **environ;
		setenv("AUTH_TYPE", _cgi_environment.env_auth_type.c_str(), 1);
    	setenv("CONTENT_LENGTH", _cgi_environment.env_content_length.c_str(), 1);
    	setenv("CONTENT_TYPE", _cgi_environment.env_content_type.c_str(), 1);
    	setenv("GATEWAY_INTERFACE", _cgi_environment.env_gateway_interface.c_str(), 1);
    	setenv("PATH_INFO", _cgi_environment.env_path_info.c_str(), 1);
    	setenv("PATH_TRANSLATED", _cgi_environment.env_path_translated.c_str(), 1);
    	setenv("QUERY_STRING", _cgi_environment.env_query_string.c_str(), 1);
    	setenv("REMOTE_ADDR", _cgi_environment.env_remote_addr.c_str(), 1);
    	setenv("REMOTE_HOST", _cgi_environment.env_remote_host.c_str(), 1);
    	setenv("REMOTE_IDENT", _cgi_environment.env_remote_ident.c_str(), 1);
    	setenv("REMOTE_USER", _cgi_environment.env_remote_user.c_str(), 1);
    	setenv("REQUEST_METHOD", _cgi_environment.env_request_method.c_str(), 1);
    	setenv("SCRIPT_NAME", _cgi_environment.env_script_name.c_str(), 1);
    	setenv("SERVER_NAME", _cgi_environment.env_server_name.c_str(), 1);
    	setenv("SERVER_PORT", _cgi_environment.env_server_port.c_str(), 1);
    	setenv("SERVER_PROTOCOL", _cgi_environment.env_server_protocol.c_str(), 1);
    	setenv("SERVER_SOFTWARE", _cgi_environment.env_server_software.c_str(), 1);

		string path = (string("www/cgi-bin") + _request.getPath());
		char* const argv[] = { const_cast<char*>(path.c_str()), NULL };
		cout << "execveing path: " << path << endl;
		sleep(5); // just here to test async/multiple clients
		execve(path.c_str(), argv, environ);

		exit(0);
	} else {

	}
}

void CgiHandler::_changeState(CgiState state, int error_code = 0) {
	_state = state;

	if (error_code != 0) {
		_error_code = error_code;
	}

	switch (_state)
	{
		case CGI_ERROR:
			break;
		case CGI_PROCESSING:
			break;
		case CGI_FINISHED:
			break;
	}

}

void CgiHandler::_parseInfo() {
	_cgi_environment.env_auth_type = "user";
    _cgi_environment.env_content_length = _request.getHeaderValue(HEADER_CONTENT_LENGTH);
    _cgi_environment.env_content_type = _request.getHeaderValue(HEADER_CONTENT_TYPE);
    _cgi_environment.env_gateway_interface = "CGI/0.0"; // pre-alpha release
    _cgi_environment.env_path_info = "NULL"; // Path after script
    _cgi_environment.env_path_translated = "NULL"; // Wot ?
    _cgi_environment.env_query_string = "NULL"; // Path after '?'
    _cgi_environment.env_remote_addr = _client_socket.getPeerIpString();
    _cgi_environment.env_remote_host = "NULL";
    _cgi_environment.env_remote_ident = "NULL"; // ?
    _cgi_environment.env_remote_user = "NULL"; // ?
    _cgi_environment.env_request_method = _request.getMethod();
    _cgi_environment.env_script_name = "NULL"; // /cgi-bin/idk.sh
    _cgi_environment.env_server_name = _client_server.getNames()[0];
    _cgi_environment.env_server_port = _client_socket.getPeerPort();
    _cgi_environment.env_server_protocol = HTTP_VERSION;
    _cgi_environment.env_server_software = SERVER_SOFTWARE;
}

void CgiHandler::update() {
	(void)_response;

	if (_state == CGI_ERROR)
		return;

	int result;

	result = waitpid(_child_pid, &_child_status, WNOHANG);
	if (result == 0) {
		// parent behaviour (Will be parsing response i think)
		// SENDING BODY ALSO? parse response when done
	} else if (result == _child_pid) {
		cout << "Cgi finished" << endl;
		// Should finish when response is ready
		_state = CGI_FINISHED;
	}
}

const CgiState &CgiHandler::getState() const {
	return _state;
}

const int &CgiHandler::getErrorCode() const {
	return _error_code;
}
