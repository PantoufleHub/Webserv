#pragma once

#include <string>
#include <iostream>

#include "EntryPoint.hpp"
#include "Socket.hpp"
#include "WebServer.hpp"

using namespace std;

class WebServer;

class ClientHandler {
   private:
	Socket _socket;
	WebServer* _server;
   public:
   	// For map
   	ClientHandler();
	ClientHandler(Socket socket, WebServer* server);
	~ClientHandler();

	void update();
};
