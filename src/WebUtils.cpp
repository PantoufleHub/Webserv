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

EntryPoint WebUtils::getSocketEntryPoint(Socket socket, bool peer) {
	int fd = socket.getFd();
	sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);

	if (peer)
		getpeername(fd, (sockaddr*)&addr, &addr_len);
	else
		getsockname(fd, (sockaddr*)&addr, &addr_len);

	EntryPoint ep;
	ep.ip = string(inet_ntoa(addr.sin_addr));
	ep.port = ntohs(addr.sin_port);

	return ep;
}
