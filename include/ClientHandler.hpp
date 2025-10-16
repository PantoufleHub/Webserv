#pragma once

#include <string>
#include <iostream>
#include <sys/wait.h>

#include "EntryPoint.hpp"
#include "Socket.hpp"
#include "WebServer.hpp"
#include "HttpRequestParser.hpp"
#include "HttpResponse.hpp"
#include "WebUtils.hpp"
#include "Logger.hpp"
#include "Constants.hpp"
#include "Macros.hpp"
#include "HttpResponse.hpp"
#include "CgiHandler.hpp"

using namespace std;

class WebServer;
class CgiHandler;

enum ClientState {
	CLIENT_READING, // Waiting for request
	CLIENT_PARSING, // Extracting info
	CLIENT_PROCESSING,  // Executing request
	CLIENT_CGIING, // Waiting for CGI
	CLIENT_ERRORING, // Generating error response
	CLIENT_RESPONDING, // Sending response
	CLIENT_DONE, // Ready to close
};

typedef struct ParsedInfo {
	const vector<VirtualServer>*	virtual_servers;
	const VirtualServer*			matching_server;
	const vector<Location>*			locations;
	const Location*					matching_location;
	const map<int, string>*			errors;
	EntryPoint						entry_point;
	string							full_path;
} ParsedInfo;

typedef struct ResponseInfo {
	string		content_type;
	bool		sent_headers;
	int 		fd_to_send; // For sending files
	size_t		bytes_sent; // For body
} ResponseInfo;

typedef struct PostInfo{
	bool	parsed;
	int		fd;
} PostInfo;

typedef struct CgiInfo {
	CgiHandler		*cgi_handler;
} CgiInfo;

class ClientHandler {
   private:
	size_t			_buffer_size;
	Socket			_socket;
	WebServer*		_server;
	ClientState		_state;
	string			_request_buffer;
	HttpRequest*	_request;
	HttpResponse	_response;
	ResponseInfo	_response_info;
	ParsedInfo		_parsed_info;
	PostInfo		_post_info;
	CgiInfo			_cgi_info;

	void _checkRequestBuffer();
	void _changeState(ClientState newState, const int stateCode);
   public:
	void _init_();
	// For map
	ClientHandler();
	ClientHandler(Socket socket, WebServer* server);
	~ClientHandler();

	void _read();

	void _validateFirstLine();
	void _findVirtualServer();
	void _findMatchingLocation();
	void _validateMatchingLocation();
	void _parse();

	void _getContentType();
	void _getResourceDirectory();
	void _getResource();
	void _postResource();
	void _deleteResource();
	void _process();

	void _fillCgiInfoPacket();
	void _cgi();

	void _error();

	void _respond();

	ClientState getState() const;

	void update();
};
