#include "WebUtils.hpp"

sockaddr_in WebUtils::createSockaddr(string ip, int port) {
	sockaddr_in sockAddr;

	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(port);
	inet_aton(ip.c_str(), &sockAddr.sin_addr);

	return sockAddr;
}

bool WebUtils::canRead(pollfd& pfd) {
	return pfd.revents & POLLIN;
}

bool WebUtils::canWrite(pollfd& pfd) {
	return pfd.revents & POLLOUT;
}
