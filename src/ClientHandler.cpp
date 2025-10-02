#include "ClientHandler.hpp"

ClientHandler::ClientHandler() {}

ClientHandler::ClientHandler(Socket socket, WebServer* server) : _socket(socket), _server(server) {
	cout	<< "New CH for fd: " << _socket.getFd() << "\n"
			<< " Server EP: " << WebUtils::getSocketEntryPoint(_socket) << "\n"
			<< " Client EP: " << WebUtils::getSocketEntryPoint(_socket, true) << "\n"
			<< endl;
	(void)_server;
	_state = PROCESSING;
	_buffer_size = 1024;
}

ClientHandler::~ClientHandler() {}

void ClientHandler::update() {
	int fd = _socket.getFd();
	pollfd& pfd = _server->getPollFd(fd);
	if (_state == PROCESSING) {
		if (WebUtils::canRead(pfd)) {
			char buffer[_buffer_size];
			ssize_t bytes_received = recv(fd, buffer, _buffer_size, 0);
			buffer[_buffer_size - 1] = '\0';

			if (bytes_received < 0) {
				cout << "Error reading from client " << fd << endl;
				_state = DONE;

			} else if (bytes_received == 0) {
				cout << "Client on socket " << fd << " disconnected" << endl;
				_state = DONE;

			} else {
				string request(buffer, bytes_received);
				cout << "Received request from client " << fd << endl;
				Logger::logRequest(request, fd);
				pfd.events = POLLOUT;
				_state = RESPONDING;
			}
		}
	}
	if (_state == RESPONDING) {
		if (WebUtils::canWrite(pfd)) {
			string http_response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
			ssize_t bytes_sent = send(fd, http_response.c_str(), http_response.size(), 0);

			if (bytes_sent < 0) {
				cout << "Error sending to client " << fd << endl;
			} else {
				cout << "Sent response to client on socket " << fd << endl;
				Logger::logResponse(http_response, fd);
			}
			_state = DONE;
		}
	}
}

ClientState ClientHandler::getState() const {
	return _state;
}
