#include "CgiHandler.hpp"

// TEMP HEADERS
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <csignal>

void CgiHandler::_init_() {
	_state = CGI_PROCESSING;
	_error_code = 0;
	_created_child = false;
	_child_pid = -1;
	_pipe_input[0] = -1;
	_pipe_input[1] = -1;
	_pipe_output[0] = -1;
	_pipe_output[1] = -1;
	_bytes_sent = 0;
	_finished_sending = false;
	_finished_reading = false;
	_cgi_body_current = 0;
	_cgi_start_time = 0;
}

CgiHandler::~CgiHandler() {
	// Not sure yet pipes?
	if (_pipe_input[0] != -1) {
		cout << "Closing _pipe_input[0]" << endl;
		_closePipeInput(0);
	}
	if (_pipe_input[1] != -1) {
		cout << "Closing _pipe_input[1]" << endl;
		_closePipeInput(1);
	}
	if (_pipe_output[0] != -1) {
		cout << "Closing _pipe_output[0]" << endl;
		_closePipeOutput(0);
	}
	if (_pipe_output[1] != -1) {
		cout << "Closing _pipe_output[1]" << endl;
		_closePipeOutput(1);
	}
}

CgiHandler::CgiHandler(	HttpResponse		&response,
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

	if (_request.getMethod() != METHOD_GET && _request.getMethod() != METHOD_POST) {
		cout << "Unimplemented CGI method" << endl;
		_changeState(CGI_ERROR, HTTP_CODE_NOT_IMPLEMENTED);
		return;
	}

	_parseInfo();
	// Not useful yet but here in case
	if (_error_code != 0) {
		cout << "CGI parsing error" << endl;
		_changeState(CGI_ERROR, HTTP_CODE_INTERNAL_SERVER_ERROR);
		return;
	}

	if (pipe(_pipe_input) == -1 || pipe(_pipe_output) == -1) {
		cout << "Pipe error" << endl;
		_changeState(CGI_ERROR, HTTP_CODE_INTERNAL_SERVER_ERROR);
		return;
	}
}

void	CgiHandler::_closePipeInput(int pipe_side) {
	close(_pipe_input[pipe_side]);
	_pipe_input[pipe_side] = -1;
}

void	CgiHandler::_closePipeOutput(int pipe_side) {
	close(_pipe_output[pipe_side]);
	_pipe_output[pipe_side] = -1;
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
		case CGI_PARSING:
			break;
		case CGI_WRITING:
			break;
	}

}

void CgiHandler::_parseInfo() {
	string request_path = _request.getPath();
	size_t question_mark_pos = request_path.find("?");
	string script_name = request_path.substr(0, question_mark_pos);
	string query_string = request_path.substr(question_mark_pos + 1);

	string pass = _client_location.getCgi();
	pass = StringUtils::trimSlashes(pass, false, true);

	size_t last_slash_pos = script_name.rfind("/"); 
	string script_raw = script_name.substr(last_slash_pos);
	script_raw = StringUtils::trimSlashes(script_raw, true, true);

	_cgi_environment.exec_path = StringUtils::pathConcatenateTrim(pass, script_raw, false, false);
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

static void setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags != -1)
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void CgiHandler::_updateCgi() {
	if (!_finished_sending) {
		size_t bytes_left = _request.getBody().size() - _bytes_sent;
		size_t to_write = (bytes_left < DEFAULT_BUFFER_SIZE) ? bytes_left : DEFAULT_BUFFER_SIZE;

		ssize_t bytes_written = write(_pipe_input[WRITE],
								_request.getBody().c_str() + _bytes_sent,
								to_write);
		if (bytes_written < 0) {
			cout << "write to CGI stdin failed" << endl;
			_changeState(CGI_ERROR, HTTP_CODE_INTERNAL_SERVER_ERROR);
			return;
		} else {
			_bytes_sent += bytes_written;
			if ((size_t)_bytes_sent >= _request.getBody().size()) {
				cout << "Finished sending to cgi" << endl;
				_closePipeInput(WRITE);
				_finished_sending = true;
				return;
			}
			return;
		}
	}

	if (!_finished_reading) {
		char buffer[DEFAULT_BUFFER_SIZE];
		ssize_t bytes_read;

		bytes_read = read(_pipe_output[READ], buffer, sizeof(buffer));
		if (bytes_read < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				return;
			} else {
				cout << "Error reading from cgi" << endl;
				_changeState(CGI_ERROR, HTTP_CODE_INTERNAL_SERVER_ERROR);
				return;
			}
		} else if (bytes_read == 0) {
			cout << "Finished reading from cgi" << endl;
			_closePipeOutput(READ);
			_finished_reading = true;
			return;
		} else {
			_cgi_output.append(buffer, bytes_read);
			return;
		}
	}
}

void CgiHandler::_parseCgiHeaders() {
	for (size_t index = 0; index < _cgi_headers.size(); index++) {
		string header = _cgi_headers[index];
		string key;
		string value;
		size_t semi_col_pos = header.find(": ");
		if (semi_col_pos == string::npos) {
			cout << "Invalid header" << endl;
			_changeState(CGI_ERROR, HTTP_CODE_INTERNAL_SERVER_ERROR);
			return;
		} 
		key = header.substr(0, semi_col_pos);
		value = header.substr(semi_col_pos + 2);
		_response.addHeader(key, value);
	}
}

void CgiHandler::_getCgiHeaders() {
	size_t index = 0;
	while (index < _cgi_output.size()) {
		size_t line_end = _cgi_output.find("\n", index);
		if (line_end == string::npos)
			break;

		string header = _cgi_output.substr(index, line_end - index);
		if (header == "" || header == "\r") { // \n or \r\n
			_cgi_body_start = line_end + 1;
			_cgi_body_current = _cgi_body_start;
			break;
		}
		_cgi_headers.push_back(header);
		index = line_end + 1;
	}
}

void CgiHandler::_parseCgiResponse() {
	if (!_finished_reading || !_finished_sending) {
		update();
	} else {
		_getCgiHeaders();
		_parseCgiHeaders();
		
		if (_state == CGI_PARSING)
			_changeState(CGI_WRITING);
	}
}

void CgiHandler::_createChildProcess() {
	string full_path = _cgi_environment.exec_path;
	struct stat sb;
	if (access(full_path.c_str(), F_OK) != 0) {
		cout << "File does not exist!" << endl;
		_changeState(CGI_ERROR, HTTP_CODE_NOT_FOUND);
		return;
	}
    if (stat(full_path.c_str(), &sb) == 0 && S_ISREG(sb.st_mode)) {
		if (access(full_path.c_str(), X_OK) != 0) {
			cout << "File can not be executed!" << endl;
			_changeState(CGI_ERROR, HTTP_CODE_FORBIDDEN);
			return;
		}
    } else {
		cout << "Not a regular file!" << endl;
		_changeState(CGI_ERROR, HTTP_CODE_FORBIDDEN);
		return;
	}
	
	cout << "Creating child process for execution of path: " << full_path << endl;
	_cgi_start_time = time(NULL);
	_child_pid = fork();
	if (_child_pid == -1) {
		cout << "Failed to fork process" << endl;
		_changeState(CGI_ERROR, HTTP_CODE_INTERNAL_SERVER_ERROR);
		return;
	} else if (_child_pid == 0) {
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

  		_closePipeInput(WRITE);
  		_closePipeOutput(READ);

  		if (dup2(_pipe_input[READ], STDIN_FILENO) == -1
			|| dup2(_pipe_output[WRITE], STDOUT_FILENO) == -1) {
			cout << "Dup failed" << endl;
			exit(EXIT_FAILURE);
		}   

  		_closePipeInput(READ);
		_closePipeOutput(WRITE);
		
		sleep(0); // just here to test async/multiple clients
		
		char* const argv[] = { const_cast<char*>(full_path.c_str()), NULL };
		// cout << "Execveing path: " << full_path << endl;
		execve(full_path.c_str(), argv, environ);

		// cout << "Execve failed, calling webserv destructor" << endl;
		_server.~WebServer();
		// cout << "Finished calling webserv destructor" << endl;
		// delete this;

		exit(EXIT_FAILURE);
	} else {
		_closePipeInput(READ);
		_closePipeOutput(WRITE);
		setNonBlocking(_pipe_input[WRITE]);
		setNonBlocking(_pipe_output[READ]);
		if (_request.getBody().empty()) {
			_closePipeInput(WRITE);
			_finished_sending = true;
		}
	}
}

void	CgiHandler::_killCgiProcess() {
	if (_child_pid > 0) {
		cout << "Killing CGI process " << _child_pid << " due to timeout" << endl;
		kill(_child_pid, SIGKILL);
		waitpid(_child_pid, &_child_status, 0);
		_child_pid = -1;
	}
}

bool	CgiHandler::_isTimedOut() const {
	if (_cgi_start_time == 0) {
		return false;
	}
	time_t current_time = time(NULL);
	time_t elapsed = current_time - _cgi_start_time;
	return elapsed > CGI_TIMEOUT;
}

void CgiHandler::_writeResponseBody() {
	_response.addBody(_cgi_output.substr(_cgi_body_current, DEFAULT_BUFFER_SIZE));
	_cgi_body_current += DEFAULT_BUFFER_SIZE;
	if (_cgi_body_current >= (_cgi_output.size() - _cgi_body_start))
		_changeState(CGI_FINISHED);
}

void CgiHandler::update() {
	(void)_response;

	if (_state == CGI_ERROR) {
		return;
	}
	if (_state == CGI_PROCESSING && _isTimedOut()){
		cout << "CGI timeout exceeded" << endl;
		_killCgiProcess();
		_changeState(CGI_ERROR, HTTP_CODE_BAD_GATEWAY);
	}
	if (_state == CGI_PROCESSING) {
		if (!_created_child) {
			_created_child = true;
			_createChildProcess();
			return;
		}

		int result;
		result = waitpid(_child_pid, &_child_status, WNOHANG);
		if (result == 0) {
			_updateCgi();
		} else if (result == _child_pid) {
			cout << "Cgi finished" << endl;
			if (WIFEXITED(_child_status)) {
            	printf("Child exited, status = %d\n", WEXITSTATUS(_child_status));
				if (WEXITSTATUS(_child_status) != 0) {
					_changeState(CGI_ERROR, HTTP_CODE_BAD_GATEWAY);
					return;
				}
            } else if (WIFSIGNALED(_child_status)) {
                printf("Child killed by signal %d\n", WTERMSIG(_child_status));
            } else if (WIFSTOPPED(_child_status)) {
                printf("Child stopped by signal %d\n", WSTOPSIG(_child_status));
            } else if (WIFCONTINUED(_child_status)) {
                printf("Child continued\n");
            }

			_changeState(CGI_PARSING);
		}
	}
	if (_state == CGI_PARSING) {
		_parseCgiResponse();
	}
	if (_state == CGI_WRITING) {
		_writeResponseBody();
	}
	
}

const CgiState &CgiHandler::getState() const {
	return _state;
}

const int &CgiHandler::getErrorCode() const {
	return _error_code;
}
