#include "ClientHandler.hpp"

void ClientHandler::_init_() {
	_state = READING;
	_buffer_size = DEFAULT_BUFFER_SIZE;
	_request = NULL;
	// -- TEMP --
	_response.setBody("text/html", "<html><body><h1>WebSaucisse surfe sur de nouveax horions</h1></body></html>");
	// ----------
	_bytes_sent = 0;
	_sent_headers = false;
	_parsed_info.virtual_servers = NULL;
	_parsed_info.matching_server = NULL;
	_parsed_info.locations = NULL;
	_parsed_info.matching_location = NULL;
	_parsed_info.errors = NULL;
	_parsed_info.full_path = "";
}

ClientHandler::ClientHandler() {}

ClientHandler::ClientHandler(Socket socket, WebServer* server) : _socket(socket), _server(server) {
	cout	<< "New CH for fd: " << _socket.getFd() << "\n"
			<< " Server EP: " << WebUtils::getSocketEntryPoint(_socket) << "\n"
			<< " Client EP: " << WebUtils::getSocketEntryPoint(_socket, true) << "\n"
			<< endl;
	_init_();
}

ClientHandler::~ClientHandler() {
	cout << "Deleting CH for fd: " << _socket.getFd() << endl;
	if (_request) {
		cout << "Deleting request for fd: " << _socket.getFd() << endl;
		delete _request;
	}
}

void ClientHandler::_changeState(ClientState newState) {
	_state = newState;
	int fd = _socket.getFd();
	pollfd& pfd = _server->getPollFd(fd);
	if (_state == READING) {
		pfd.events = POLLIN;
	}
	else if (_state == PARSING) {
		pfd.events = 0;
	}
	else if (_state == PROCESSING) {
		pfd.events = 0;
	}
	else if (_state == CGIING) {
		pfd.events = 0;
	}
	else if (_state == ERRORING) {
		pfd.events = 0;
		// Reset response
		_response.setBody("text/html", "");
	}
	else if (_state == RESPONDING) {
		pfd.events = POLLOUT;
	}
	else if (_state == DONE) {
		pfd.events = 0;
	}
}

void ClientHandler::_checkRequestBuffer() {
	// CheckDataIn still needs to handle chunked transfer encoding
	int request_length = HttpRequestParser::checkDataIn(_request_buffer);
	if (request_length > 0) {
		cout << "Full request received on socket " << _socket.getFd() << endl;
		_request = HttpRequestParser::strToHttpRequest(_request_buffer.substr(0, request_length));
		_changeState(PARSING);
		Logger::logRequest(_request->toString(), _socket.getFd());
	} else if (request_length < 0) {
		cout << "Bad request on socket " << _socket.getFd() << endl;
		_changeState(DONE); // Send error response?
	} else {
		// Incomplete request, keep reading
		if (_request_buffer.size() > MAX_REQUEST_SIZE) {
			cout << "Request too large on socket " << _socket.getFd() << endl;
			_changeState(DONE); // Send error response?
		}
	}
}

void ClientHandler::_read() {
	int fd = _socket.getFd();
	pollfd& pfd = _server->getPollFd(fd);
	if (!WebUtils::canRead(pfd))
		return;

	char buffer[_buffer_size + 1];
	ssize_t bytes_received = recv(fd, buffer, _buffer_size, 0);
	buffer[_buffer_size] = '\0';

	if (bytes_received < 0) {
		cout << "Error reading from client " << fd << endl;
		_changeState(DONE);

	} else if (bytes_received == 0) {
		cout << "Client on socket " << fd << " disconnected" << endl;
		_changeState(DONE);
		
	} else {
		string data_read(buffer, bytes_received);
		cout << "Received " << bytes_received << " bytes from client " << fd << endl;
		_request_buffer += data_read;
		_checkRequestBuffer();
	}
}

void ClientHandler::_validateFirstLine() {
	const string method = _request->getMethod();
	const string path = _request->getPath();
	const string http_version = _request->getHttpVersion();

	if (method != METHOD_GET && method != METHOD_POST && method != METHOD_DELETE) {
		_response.setStatusCode( HTTP_CODE_METHOD_NOT_ALLOWED);
		_changeState(ERRORING);
		return;
	}

	// Prevent directory traversal attacks
	if (path.find("..") != string::npos || path.find("~") != string::npos) {
		_response.setStatusCode( HTTP_CODE_BAD_REQUEST);
		_changeState(ERRORING);
		return;
	}

	if (http_version != "HTTP/1.1") {
		_response.setStatusCode( HTTP_CODE_HTTP_VERSION_NOT_SUPPORTED);
		_changeState(ERRORING);
		return;
	}
}

void ClientHandler::_findVirtualServer() {
	// find correct or fallback on a default
	// Default should be the first server block with matching ip:port
	// or one tagged with default_serv i think that's it

	// STILL NEEDS WORK
	// For example localhost should be parsed to 127.0.0.1
	// and i can't think of other stuff right now
	const vector<VirtualServer>& servers = *_parsed_info.virtual_servers;
	const EntryPoint& entry_point = _parsed_info.entry_point;
	const string& host = _request->getHeaderValue(HEADER_HOST);
	int serv_index = 0;

	if (host == "")
		return;

	cout << "\nLooking for virtual server, set default to: " <<  servers[serv_index].getNames()[0] << endl;
	cout << "Looking for host: " << host << endl;
	cout << "on entry point: " << entry_point.ip << ":" << entry_point.port << endl;
	bool found_default = false;

	for (size_t i = 0; i < servers.size(); i++) {
		const VirtualServer& server = servers[i];
		if (!EpInEps(entry_point, server.getEntryPoints()))
			continue;
		if (!found_default)
			serv_index = i, found_default = true;
		if (StringUtils::strInStrs(host, server.getNames())) {
			cout << "Found matching server: " << server.getNames()[0] << endl;
			_parsed_info.matching_server = &server;
			return;
		}
	}
	cerr << "No server matched, using default: " << servers[serv_index].getNames()[0] << endl;
	_parsed_info.matching_server = &servers[serv_index];
	return;
}

void ClientHandler::_findMatchingLocation() {
	const vector<Location>& locations = *_parsed_info.locations;
	const string path = _request->getPath();
	bool location_found = false;
	size_t best_match_index = 0;
	size_t best_len = 0;

	for (size_t i = 0; i < locations.size(); ++i) {
		const Location& location = locations[i];
		const string& candidate = location.getNames()[0];
		if (path.size() >= candidate.size() && path.compare(0, candidate.size(), candidate) == 0 &&
		    candidate.size() > best_len) {
			best_match_index = i;
			best_len = candidate.size();
			location_found = true;
		}
	}

	if (location_found)
		_parsed_info.matching_location = &locations[best_match_index];
}

void ClientHandler::_validateMatchingLocation() {
	const Location& matching_location = *_parsed_info.matching_location;
	const string method = _request->getMethod();

	cout << "DEBUG: Validating method '" << method << "' for location '" << matching_location.getNames()[0] << "'" << endl;
	
	if (!matching_location.checkMethod(method)) {
		cout << "DEBUG: Method '" << method << "' not allowed, setting 405" << endl;
		_response.setStatusCode(HTTP_CODE_METHOD_NOT_ALLOWED);
		_changeState(ERRORING);
		return;
	}
	cout << "Best match for path '" << _request->getPath() << "' is location: " << matching_location.getNames()[0] << endl;

	return;
}

void ClientHandler::_parse() {
	// PARSE REQUEST
	int fd = _socket.getFd();
	cout << "Parsing request from client on socket " << fd << endl;
	
	_validateFirstLine();
	if (_state != PARSING)
		return;
	
	_parsed_info.virtual_servers = &_server->getVirtualServers();
	_parsed_info.entry_point = WebUtils::getSocketEntryPoint(_socket);
	_findVirtualServer();
	if (_state != PARSING)
		return;
	
	if (!_parsed_info.matching_server) {
		_response.setStatusCode(HTTP_CODE_INTERNAL_SERVER_ERROR);
		_changeState(ERRORING);
		return;
	}
	_parsed_info.errors = &_parsed_info.matching_server->getErrors();

	string path = _request->getPath();
	string full_path = _parsed_info.matching_server->getRoot() + (path[0] == '/' ? path.substr(1) : path);
	_parsed_info.full_path = full_path;

	_parsed_info.locations = &_parsed_info.matching_server->getLocations();
	if (_parsed_info.locations->empty()) {
		_response.setStatusCode(HTTP_CODE_NOT_FOUND);
		_changeState(ERRORING);
		return;
	}

	_findMatchingLocation();
	if (_parsed_info.matching_location == NULL) {
		_response.setStatusCode(HTTP_CODE_NOT_FOUND);
		_changeState(ERRORING);
		return;
	}

	_validateMatchingLocation();
	if (_state != PARSING)
		return;

	_changeState(PROCESSING);
}

void ClientHandler::_process() {
	// PROCESS GET/POST/DELETE
	int fd = _socket.getFd();
	// pollfd& pfd = _server->getPollFd(fd);
	if  (!_request || !_request->isValid()) {
		cout << "Invalid request from client on socket " << fd << endl;
		_changeState(DONE); // Need to send error response
		return;
	}
	cout << "Processing request from client on socket " << fd << endl;
	_changeState(RESPONDING);
}

void ClientHandler::_cgi() {
	// HANDLE CGI
	int fd = _socket.getFd();

	cout << "Handling CGI for request from client on socket " << fd << endl;

	_changeState(RESPONDING);
}

void ClientHandler::_error() {
	// HANDLE ERRORS
	int fd = _socket.getFd();

	cout << "Handling Error for request from client on socket " << fd << endl;

	_changeState(RESPONDING);
}

void ClientHandler::_respond() {
	int fd = _socket.getFd();
	pollfd& pfd = _server->getPollFd(fd);
	
	if (!WebUtils::canWrite(pfd))
		return;
	
	ssize_t bytes_sent;
	string chunk_to_send;

	if (!_sent_headers) {
		chunk_to_send = _response.getHeadersString();
		bytes_sent = send(fd, chunk_to_send.c_str(), chunk_to_send.size(), 0);
		if (bytes_sent <= 0) {
			cout << "Error sending headers to client " << fd << endl;
			_changeState(DONE);
			return;
		}
		cout << "Sent " << chunk_to_send << " to client " << fd << endl;
		_sent_headers = true;
		cout << "Headers sent to client on socket " << fd << endl;
		return;
	}

	size_t chunk_bytes = _response.getBodyChunk(chunk_to_send, _bytes_sent, _buffer_size);
	if (chunk_bytes == 0) {
		cout << "No more body to send to client on socket " << fd << endl;
		_changeState(DONE);
		return;
	}
	bytes_sent = send(fd, chunk_to_send.c_str(), chunk_to_send.size(), 0);
	_bytes_sent += bytes_sent;
	cout << "Sent " << chunk_to_send << " to client " << fd << endl;
	if (bytes_sent <= 0) {
		cout << "Error sending to client " << fd << endl;
		_changeState(DONE);
		return;
	}
	else if (chunk_to_send.size() < _buffer_size || _bytes_sent >= _response.getBodySize())
	{
		cout << "Finshed sending response to client on socket " << fd << endl;
		Logger::logResponse(_response.toString(), fd);
		_changeState(DONE);
		return;
	}
}

void ClientHandler::update() {
	if (_state == READING)
		_read();
	if (_state == PARSING)
		_parse();
	if (_state == PROCESSING)
		_process();
	if (_state == CGIING)
		_cgi();
	if (_state == ERRORING)
		_error();
	if (_state == RESPONDING)
		_respond();
}

ClientState ClientHandler::getState() const {
	return _state;
}
