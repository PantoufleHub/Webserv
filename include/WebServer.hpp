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

using namespace std;

class WebServer {
   private:
	vector<pollfd> _pollfds;
	size_t _nb_listening_fds; // reservers the first n pollfds for listening sockets
	// map<int, ClientHandler> _clients; client fd, handler 
	// coming soon :)
	vector<VirtualServer> _virtual_servers;
	
	
	void _openAllServerSockets();
	void _openListeningSocket(string ip, int port, int backlog);
	void _addListeningSocketFd(pollfd& pollfd);
	void _openClientSocket(int listening_socket);
	void _updateClientSockets();
	void _updateListeningSockets();

   public:
	WebServer(const string& config_file);
	~WebServer();

	void run();
	void display() const;
};