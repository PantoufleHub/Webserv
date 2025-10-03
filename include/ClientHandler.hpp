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
	READING,
	PROCESSING,
	RESPONDING,
	DONE,
};

class ClientHandler {
   private:
	size_t _buffer_size;
	Socket _socket; // You can find the useful fd in here
	WebServer* _server;
	ClientState _state;
	string _request_buffer;
	HttpRequest *_request;
	HttpResponse *_response;
	string _response_buffer;
	bool _sent_headers;
	size_t _bytes_sent;

	void _checkRequestBuffer();
   public:
   	// For map
   	ClientHandler();
	ClientHandler(Socket socket, WebServer* server);
	~ClientHandler();

	void _read();
	void _process();
	void _respond();
	void changeState(ClientState newState);

	ClientState getState() const;

	void update();
};
