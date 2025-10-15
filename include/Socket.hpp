#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <arpa/inet.h>

using namespace std;

/// @brief Represents a socket (yes)
class Socket {
   private:
	int _fd;
	sockaddr_in _socket_address;
	sockaddr_in _peer_socket_address;

   public:
	// for map
	Socket();
	Socket(int fd);

	string getIpString() const;
	string getPeerIpString() const;
	uint16_t getPort() const;
	uint16_t getPeerPort() const;

	const sockaddr_in& getSockAddr() const;
	const sockaddr_in& getPeerSockAddr() const;
	const int& getFd() const;
};
