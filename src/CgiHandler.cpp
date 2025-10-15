#include "CgiHandler.hpp"

// TEMP HEADERS
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>

void CgiHandler::_init_() {
	_state = CGI_PROCESSING;
	_error_code = 0;
	_created_child = false;
}

CgiHandler::CgiHandler(	const HttpResponse	&response,
						const HttpRequest	&request,
						const WebServer		&server,
						const VirtualServer	&client_server,
						const Location		&client_location,
						const Socket 		&client_socket) : 
						_response(response),
						_request(request),
						_server(server),
						_client_server(client_server),
						_client_location(client_location),
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
	string request_path = _request.getPath();
	size_t question_mark_pos = request_path.find("?");
	string script_name = request_path.substr(0, question_mark_pos);
	string query_string = request_path.substr(question_mark_pos + 1);

	const map<string, vector<string> > caca = _client_location.getCgi();
	string pass = caca.at("cgi_pass")[0];
	pass = pass.substr(0, pass.rfind("/")); // remove last /

	size_t last_slash_pos = script_name.rfind("/"); 
	string script_raw = script_name.substr(last_slash_pos);
	string full_path = pass + script_raw;

	_cgi_environment.exec_path = full_path;
	_cgi_environment.script_raw_name = script_raw;

	_cgi_environment.env_auth_type = "user";
    _cgi_environment.env_content_length = _request.getHeaderValue(HEADER_CONTENT_LENGTH);
    _cgi_environment.env_content_type = _request.getHeaderValue(HEADER_CONTENT_TYPE);
    _cgi_environment.env_gateway_interface = "CGI/0.0"; // pre-alpha release
    _cgi_environment.env_path_info = "NULL"; // Path after script
    _cgi_environment.env_path_translated = "NULL"; // ?
    _cgi_environment.env_query_string = query_string;
    _cgi_environment.env_remote_addr = _client_socket.getPeerIpString();
    _cgi_environment.env_remote_host = "NULL"; // ?
    _cgi_environment.env_remote_ident = "NULL"; // ?
    _cgi_environment.env_remote_user = "NULL"; // ?
    _cgi_environment.env_request_method = _request.getMethod();
    _cgi_environment.env_script_name = script_name;
    _cgi_environment.env_server_name = _client_server.getNames()[0];
    _cgi_environment.env_server_port = _client_socket.getPeerPortString();
    _cgi_environment.env_server_protocol = HTTP_VERSION;
    _cgi_environment.env_server_software = SERVER_SOFTWARE;
}

void CgiHandler::_createChildProcess() {
	_created_child = true;

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

		string full_path = _cgi_environment.exec_path;
		char* const argv[] = { const_cast<char*>(full_path.c_str()), NULL };
		cout << "Execveing path: " << full_path << endl;
		// WTF? memoryysdfa f;slakdjfakjfhlkdjfhadkjfhaassdlkjhljasahsdflkjasdhflkassjdfhas
		sleep(0); // just here to test async/multiple clients
		execve(full_path.c_str(), argv, environ);
		cout << "Execve failed" << endl;
		_server.~WebServer();
		// (void)_server;
		cout << "Finished calling webserv destructor" << endl;
		close(_pipe[0]);
		close(_pipe[1]);
		// delete this;
		exit(EXIT_FAILURE);
	} else {
		close(_pipe[0]);
		close(_pipe[1]);
	}
}

void CgiHandler::update() {
	(void)_response;

	if (_state == CGI_ERROR) {

		return;
	}
	if (_state == CGI_PROCESSING) {
		if (!_created_child) {
			_createChildProcess();
		}

		int result;
		result = waitpid(_child_pid, &_child_status, WNOHANG);
		if (result == 0) {
			// parent behaviour (Will be parsing response i think)
			// SENDING BODY ALSO? parse response when done
		} else if (result == _child_pid) {
			cout << "Cgi finished" << endl;
			// If bad status BAD_GATEWAY
			// Should finish when response is ready
			_changeState(CGI_FINISHED);
		}
	}
	
}

const CgiState &CgiHandler::getState() const {
	return _state;
}

const int &CgiHandler::getErrorCode() const {
	return _error_code;
}
