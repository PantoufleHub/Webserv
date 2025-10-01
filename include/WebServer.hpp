#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <poll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <ConfigParser.hpp>
#include <VirtualServer.hpp>
#include <Macros.hpp>
#include <SocketUtils.hpp>
#include <Logger.hpp>

using namespace std;

class WebServer {
   private:
	vector<pollfd> _fds;
	vector<VirtualServer> _virtual_servers;

   public:
	WebServer(const string& config_file);
	~WebServer();

	int _openClientSocket(int listening_socket);

	void run();
	void display() const;
};