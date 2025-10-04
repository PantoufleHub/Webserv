#pragma once

#include <string>
#include <iostream>

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

using namespace std;

class WebServer;

enum ClientState {
	READING, // Waiting for request
	PARSING, // Extracting info
	PROCESSING,  // Executing request
	CGIING, // Waiting for CGI
	ERRORING, // Generating error response
	RESPONDING, // Sending response
	DONE, // Ready to close
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

class ClientHandler {
   private:
	size_t _buffer_size;
	Socket _socket;
	WebServer* _server;
	ClientState _state;
	string _request_buffer;
	HttpRequest *_request;
	HttpResponse _response;
	string _response_buffer;
	bool _sent_headers;
	size_t _bytes_sent;
	ParsedInfo _parsed_info;

	void _checkRequestBuffer();
	void _changeState(ClientState newState);
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

	void _process();

	void _cgi();

	void _error();

	void _respond();

	ClientState getState() const;

	void update();
};
