#include "ClientHandler.hpp"

ClientHandler::ClientHandler() {}

ClientHandler::ClientHandler(Socket socket, WebServer* server) : _socket(socket), _server(server) {
	cout	<< "New CH for fd: " << _socket.getFd() << "\n"
			<< " Server EP: " << WebUtils::getSocketEntryPoint(_socket) << "\n"
			<< " Client EP: " << WebUtils::getSocketEntryPoint(_socket, true) << "\n"
			<< endl;
	_state = READING;
	_buffer_size = DEFAULT_BUFFER_SIZE;
	_request = NULL;
	_response = NULL;
	_bytes_sent = 0;
}

ClientHandler::~ClientHandler() {
	cout << "Deleting CH for fd: " << _socket.getFd() << endl;
	if (_request) {
		cout << "Deleting request for fd: " << _socket.getFd() << endl;
		delete _request;
	}
}

void ClientHandler::changeState(ClientState newState) {
	_state = newState;
	int fd = _socket.getFd();
	pollfd& pfd = _server->getPollFd(fd);
	if (_state == READING) {
		pfd.events = POLLIN;
	}
	else if (_state == PROCESSING) {
		pfd.events = 0;
	}
	else if (_state == RESPONDING) {
		// -- TEMP --
		_response = new HttpResponse(200);
		_response->setBody("text/html", "<html><body><h1>WebSaucisse surf sur de nouveax horions</h1></body></html>");
		// ----------
		_response_buffer = _response->toString();
		_bytes_sent = 0;
		pfd.events = POLLOUT;
	}
	else if (_state == DONE) {
		pfd.events = 0;
	}
}

void ClientHandler::_checkRequestBuffer() {
	int request_length = HttpRequestParser::checkDataIn(_request_buffer);
	if (request_length > 0) {
		cout << "Full request received on socket " << _socket.getFd() << endl;
		_request = HttpRequestParser::strToHttpRequest(_request_buffer.substr(0, request_length));
		changeState(PROCESSING);
		Logger::logRequest(_request->toString(), _socket.getFd());
	} else if (request_length < 0) {
		cout << "Bad request on socket " << _socket.getFd() << endl;
		changeState(DONE); // Send error response?
	} else {
		// Incomplete request, keep reading
		if (_request_buffer.size() > MAX_REQUEST_SIZE) {
			cout << "Request too large on socket " << _socket.getFd() << endl;
			changeState(DONE); // Send error response?
		}
	}
}

void ClientHandler::_read() {
	int fd = _socket.getFd();
	pollfd& pfd = _server->getPollFd(fd);
	if (WebUtils::canRead(pfd)) {
			char buffer[_buffer_size + 1];
			ssize_t bytes_received = recv(fd, buffer, _buffer_size, 0);
			buffer[_buffer_size] = '\0';
		if (bytes_received < 0) {
			cout << "Error reading from client " << fd << endl;
			changeState(DONE);

		} else if (bytes_received == 0) {
			cout << "Client on socket " << fd << " disconnected" << endl;
			changeState(DONE);

		} else {
			string data_read(buffer, bytes_received);
			cout << "Received " << bytes_received << " bytes from client " << fd << endl;
			_request_buffer += data_read;
			_checkRequestBuffer();
		}
	}
}

void ClientHandler::_process() {
	// PROCESS GET/POST/DELETE
	int fd = _socket.getFd();
	// pollfd& pfd = _server->getPollFd(fd);
	if  (!_request || !_request->isValid()) {
		cout << "Invalid request from client on socket " << fd << endl;
		changeState(DONE);
		return;
	}
	cout << "Processing request from client on socket " << fd << endl;
	changeState(RESPONDING);
}

void ClientHandler::_respond() {
	int fd = _socket.getFd();
	pollfd& pfd = _server->getPollFd(fd);
	if (WebUtils::canWrite(pfd)) {
		string chunk_to_send = _response_buffer.substr(_bytes_sent, _buffer_size);
		ssize_t bytes_sent = send(fd, chunk_to_send.c_str(), chunk_to_send.size(), 0);
		_bytes_sent += bytes_sent;
		cout << "Sent " << bytes_sent << " bytes to client " << fd << endl;
		if (bytes_sent <= 0) {
			cout << "Error sending to client " << fd << endl;
			changeState(DONE);
		}
		else if (_bytes_sent >= _response_buffer.size())
		{
			cout << "Finshed sending response to client on socket " << fd << endl;
			Logger::logResponse(_response->toString(), fd);
			changeState(DONE);
		}
	}
}

void ClientHandler::update() {
	if (_state == READING)
		_read();
	if (_state == PROCESSING)
		_process();
	if (_state == RESPONDING)
		_respond();
}

ClientState ClientHandler::getState() const {
	return _state;
}
