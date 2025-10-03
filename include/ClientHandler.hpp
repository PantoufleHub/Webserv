#pragma once

#include <string>
#include <iostream>

#include "EntryPoint.hpp"
#include "Socket.hpp"
#include "WebServer.hpp"
#include "HttpRequestParser.hpp"

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

	void _checkRequestBuffer();
   public:
   	// For map
   	ClientHandler();
	ClientHandler(Socket socket, WebServer* server);
	~ClientHandler();

	ClientState getState() const;

	void update();
};
