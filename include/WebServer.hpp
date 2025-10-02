#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <poll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "ConfigParser.hpp"
#include "VirtualServer.hpp"
#include "Macros.hpp"
#include "WebUtils.hpp"
#include "Logger.hpp"
#include "EntryPoint.hpp"
#include "Socket.hpp"
#include "ClientHandler.hpp"
#include "Constants.hpp"

class ClientHandler;

using namespace std;

class WebServer {
   private:
	vector<VirtualServer> _virtual_servers;
	vector<pollfd> _pollfds;
	vector<Socket> _listening_sockets;

	// NEED TO FIND A WAY TO SYNC CLIENTS AND POLLFDS
	// REVAMP SOCKET TO HOLD POLLFDS?
	// SEND POLLFD IN UPDATE? OR SEND SOCKET?
	// CANT HOLD POLLFD IN CLIENTHANDLER, NEEDS TO BE IN WEBSERVER
	map<int, ClientHandler> _clients; // _pollfds.fd, ClientHandler
	
	void _openListeningSocket(string ip, int port);
	void _openAllServerSockets();
	void _updateListeningSockets();
	void _openClientSocket(int listening_socket);
	void _updateClientSockets();
	void _garbageCollectClients();

   public:
	WebServer(const string& config_file);
	~WebServer();

	pollfd& getPollFd(int fd);
	void removePollFd(int fd);

	void run();
	void display() const;
};