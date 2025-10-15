#include "Socket.hpp"

Socket::Socket() : _fd(-1) {}

Socket::Socket(int fd) : _fd(fd) {
	socklen_t addrlen = sizeof(sockaddr_in);
	getsockname(_fd, (sockaddr*)&_socket_address, &addrlen);
	getpeername(_fd, (sockaddr*)&_peer_socket_address, &addrlen);

	// Announcing socket creation!
	// cout << "\n --- New socket " << _fd << " ---\n"
	//      << "  Linking " << inet_ntoa(_socket_address.sin_addr) << ":" << ntohs(_socket_address.sin_port)
	//      << "\n  to      " << inet_ntoa(_peer_socket_address.sin_addr) << ":" << ntohs(_peer_socket_address.sin_port)
	//      << endl
	//      << endl;
}

string Socket::getIpString() const {
	return string(inet_ntoa(_socket_address.sin_addr));
}

string Socket::getPeerIpString() const {
	return string(inet_ntoa(_peer_socket_address.sin_addr));
}

uint16_t Socket::getPort() const {
	return ntohs(_socket_address.sin_port);
}

string Socket::getPortString() const {
	stringstream ret_strstr;

	ret_strstr << getPort();

	return ret_strstr.str();
}

uint16_t Socket::getPeerPort() const {
	return ntohs(_peer_socket_address.sin_port);
}

string Socket::getPeerPortString() const {
	stringstream ret_strstr;

	ret_strstr << getPeerPort();

	return ret_strstr.str();
}

const sockaddr_in& Socket::getSockAddr() const {
	return _socket_address;
}

const sockaddr_in& Socket::getPeerSockAddr() const {
	return _peer_socket_address;
}

const int& Socket::getFd() const {
	return _fd;
}
