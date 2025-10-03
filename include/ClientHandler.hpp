#pragma once

#include <string>
#include <iostream>

#include "EntryPoint.hpp"
#include "Socket.hpp"
#include "WebServer.hpp"

using namespace std;

class WebServer;

enum ClientState {
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
   public:
   	// For map
   	ClientHandler();
	ClientHandler(Socket socket, WebServer* server);
	~ClientHandler();

	ClientState getState() const;

	void update();
};
