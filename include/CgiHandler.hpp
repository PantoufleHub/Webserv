#pragma once

#include "WebServer.hpp"
#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "Socket.hpp"

#include <iostream>

class WebServer;

using namespace std;

typedef enum CgiState {
	CGI_PROCESSING,
	CGI_FINISHED,
	CGI_ERROR,
} CgiState;

class CgiHandler{
   private:
	CgiState			_state;
	int					_child_pid;
	int					_child_status;
	int					_pipe[2];
	int					_error_code;
	const HttpResponse	&_response;
	const HttpRequest	&_request;
	const WebServer		&_web_server;
	const VirtualServer	&_client_server;
	const Socket		&_client_socket;

   public:
	void _init_();
	// BLEEEEUHGGHGAAAHDKHLAJK
	CgiHandler(	const HttpResponse	&response,
				const HttpRequest	&request,
				const WebServer		&web_server,
				const VirtualServer	&client_server,
				const Socket		&client_socket);

	void update();
	void parseInfo();

	const CgiState &getState() const;
	const int &getErrorCode() const;
};
