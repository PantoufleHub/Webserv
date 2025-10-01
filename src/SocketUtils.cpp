#include "SocketUtils.hpp"

sockaddr_in SocketUtils::createSockaddr(string ip, int port) {
	sockaddr_in sockAddr;

	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(port);
	inet_aton(ip.c_str(), &sockAddr.sin_addr);

	return sockAddr;
}
