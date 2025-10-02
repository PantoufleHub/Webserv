#include "ClientHandler.hpp"

ClientHandler::ClientHandler() {}

ClientHandler::ClientHandler(Socket socket, WebServer* server) : _socket(socket), _server(server) {
	cout	<< "New CH for fd: " << _socket.getFd() << "\n"
			<< " Server EP: " << WebUtils::getSocketEntryPoint(_socket) << "\n"
			<< " Client EP: " << WebUtils::getSocketEntryPoint(_socket, true) << "\n"
			<< endl;
	// cout << "Server info: " << endl;
	// _server->display();
	(void)_server;
	_state = PROCESSING;
	_buffer_size = 1024;
}

ClientHandler::~ClientHandler() {}

void ClientHandler::update() {
	pollfd& pfd = _server->getPollFd(_socket.getFd());
	if (_state == PROCESSING) {
		if (WebUtils::canRead(pfd)) {
			char buffer[_buffer_size];
			ssize_t bytes_received = recv(_socket.getFd(), buffer, _buffer_size, 0);
			buffer[_buffer_size - 1] = '\0';

			if (bytes_received < 0) {
				cout << "Error reading from client " << _socket.getFd() << endl;
				_state = DONE;

			} else if (bytes_received == 0) {
				cout << "Client on socket " << _socket.getFd() << " disconnected" << endl;
				_state = DONE;

			} else {
				string request(buffer, bytes_received);
				cout << "Received request from client " << _socket.getFd() << endl;
				Logger::logRequest(request, _socket.getFd());
				pfd.events = POLLOUT;
				_state = RESPONDING;
			}
		}
	}
	if (_state == RESPONDING) {
		if (WebUtils::canWrite(pfd)) {
			string http_response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
			ssize_t bytes_sent = send(_socket.getFd(), http_response.c_str(), http_response.size(), 0);

			if (bytes_sent < 0) {
				cout << "Error sending to client " << _socket.getFd() << endl;
			} else {
				cout << "Sent response to client on socket " << _socket.getFd() << endl;
				Logger::logResponse(http_response, _socket.getFd());
			}
			_state = DONE;
		}
	}
}

ClientState ClientHandler::getState() const {
	return _state;
}
