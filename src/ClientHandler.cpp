#include "ClientHandler.hpp"

ClientHandler::ClientHandler() {}

ClientHandler::ClientHandler(Socket socket, WebServer* server) : _socket(socket), _server(server) {
	cout	<< "New CH for fd: " << _socket.getFd() << "\n"
			<< " Server EP: " << WebUtils::getSocketEntryPoint(_socket) << "\n"
			<< " Client EP: " << WebUtils::getSocketEntryPoint(_socket, true) << "\n"
			<< endl;
	(void)_server;
	_state = READING;
	_buffer_size = DEFAULT_BUFFER_SIZE;
	_request = NULL;
}

ClientHandler::~ClientHandler() {
	if (_request)
		delete _request;
}

void ClientHandler::_checkRequestBuffer() {
	int request_length = HttpRequestParser::checkDataIn(_request_buffer);
	if (request_length > 0) {
		cout << "Full request received on socket " << _socket.getFd() << endl;
		_request = HttpRequestParser::strToHttpRequest(_request_buffer.substr(0, request_length));
		_state = PROCESSING;
		Logger::logRequest(_request->toString(), _socket.getFd());
	} else if (request_length < 0) {
		cout << "Bad request on socket " << _socket.getFd() << endl;
		_state = DONE; // Send error response?
	} else {
		// Incomplete request, keep reading
		if (_request_buffer.size() > MAX_REQUEST_SIZE) {
			cout << "Request too large on socket " << _socket.getFd() << endl;
			_state = DONE; // Send error response?
		}
	}
}

void ClientHandler::update() {
	int fd = _socket.getFd();
	pollfd& pfd = _server->getPollFd(fd);
	if (_state == READING) {
		if (WebUtils::canRead(pfd)) {
			char buffer[_buffer_size + 1];
			ssize_t bytes_received = recv(fd, buffer, _buffer_size, 0);
			buffer[_buffer_size] = '\0';
			if (bytes_received < 0) {
				cout << "Error reading from client " << fd << endl;
				_state = DONE;

			} else if (bytes_received == 0) {
				cout << "Client on socket " << fd << " disconnected" << endl;
				_state = DONE;

			} else {
				string data_read(buffer, bytes_received);
				cout << "Received " << bytes_received << " bytes from client " << fd << endl;
				_request_buffer += data_read;
				_checkRequestBuffer();
			}
		}
	}
	if (_state == PROCESSING) {
		// PROCESS GET/POST/DELETE
		if  (!_request || !_request->isValid()) {
			cout << "Invalid request from client on socket " << fd << endl;
			_state = DONE;
			return;
		}
		cout << "Processing request from client on socket " << fd << endl;
		_state = RESPONDING;
		pfd.events = POLLOUT;
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
