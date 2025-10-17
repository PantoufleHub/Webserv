#pragma once

#include "WebServer.hpp"
#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "Socket.hpp"

#include <iostream>
#include <cstring>
#include <vector>
#include <sys/stat.h>

class WebServer;

using namespace std;

typedef enum CgiState {
	CGI_PROCESSING,
	CGI_PARSING,
	CGI_WRITING,
	CGI_FINISHED,
	CGI_ERROR,
} CgiState;

typedef struct CgiEnvironment {
	string script_raw_name;
	string exec_path;
	string env_auth_type;
    string env_content_length;
    string env_content_type;
    string env_gateway_interface;
    string env_path_info;
    string env_path_translated;
    string env_query_string;
    string env_remote_addr;
    string env_remote_host;
    string env_remote_ident;
    string env_remote_user;
    string env_request_method;
    string env_script_name;
    string env_server_name;
    string env_server_port;
    string env_server_protocol;
    string env_server_software;
} CgiEnvironment;

class CgiHandler{
   private:
	bool					_created_child;
	CgiState				_state;
	int						_child_pid;
	int						_child_status;
	int						_pipe_input[2];
	int						_pipe_output[2];
	ssize_t					_bytes_sent;
	bool					_finished_sending;
	bool					_finished_reading;
	string					_cgi_output;
	size_t					_cgi_body_start;
	size_t					_cgi_body_current;
	vector<string>			_cgi_headers;
	int						_error_code;
	HttpResponse&			_response;
	const HttpRequest&		_request;
	const WebServer&		_server;
	const VirtualServer&	_client_server;
	const Location&			_client_location;
	const Socket&			_client_socket;
	CgiEnvironment			_cgi_environment;

	void			_closePipeInput(int pipeSide);
	void			_closePipeOutput(int pipeSide);
	void			_parseInfo();
	void			_updateCgi();
	void			_getCgiHeaders();
	void			_parseCgiHeaders();
	void			_parseCgiResponse();
	void			_writeResponseBody();
	void			_changeState(CgiState state, int error_code);
	void			_createChildProcess();

   public:
	void _init_();
	
	CgiHandler(	HttpResponse&			response,
				const HttpRequest&		request,
				const WebServer&		server,
				const VirtualServer&	client_server,
				const Location&			client_location,
				const Socket&			client_socket);
	~CgiHandler();

	void update();

	const CgiState &getState() const;
	const int &getErrorCode() const;
};
