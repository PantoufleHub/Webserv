#include "ClientHandler.hpp"

void ClientHandler::_init_() {
	_state = READING;
	_buffer_size = DEFAULT_BUFFER_SIZE;
	_request = NULL;
	// -- TEMP --
	_response.setBody("text/html", "<html><body><h1>WebSaucisse surfe sur de nouveax horions</h1></body></html>");
	// ----------

	// Response info init
	_response_info.bytes_sent = 0;
	_response_info.sent_headers = false;
	_response_info.fd_to_send = -1;

	// Parsed info init
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
	if (_response_info.fd_to_send != -1) {
		cout << "Closing file descriptor to send: " << _response_info.fd_to_send << endl;
		close(_response_info.fd_to_send);
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
		cout	<< "Preparing to respond to client on socket " << fd << "\n"
				<< "Fd to send: " << _response_info.fd_to_send << endl;
		pfd.events = POLLOUT;

		// Set headers before sending
		if (_response_info.fd_to_send == -1)
			_response.addHeader(HEADER_CONTENT_LENGTH, StringUtils::sizetToString(_response.getBodySize()));
		else
			_response.addHeader(HEADER_TRANSFER_ENCODING, ENCODING_CHUNKED);
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
	cout << "No server matched, using default: " << servers[serv_index].getNames()[0] << endl;
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

void ClientHandler::_getResourceDirectory() {
	const Location& location = *_parsed_info.matching_location;
	const string path = _parsed_info.full_path;
	bool auto_index = location.getAutoIndex();
	
	if (auto_index) {
		cout << "Autoindex is enabled, generating page" << endl;
		HttpUtils::getAutoIndexPage(_response, location, path);
		_changeState(RESPONDING);
		return;
	} else {
		cout << "Autoindex is disabled, searching for index files..." << endl;
		ifstream file;
		for (size_t i = 0; i < location.getIndexes().size(); i++) {
			string test_path = path + location.getIndexes()[i];
			cout << "Testing index file: " << test_path << endl;
			file.open(test_path.c_str());
			if (file.is_open()) {
				cout << "Found index file: " << test_path << endl;
				_response_info.fd_to_send = open(test_path.c_str(), O_RDONLY);
				if (_response_info.fd_to_send < 0) {
					cerr << "Error opening file descriptor for: " << test_path << endl;
					_response.setStatusCode(HTTP_CODE_FORBIDDEN);
					_changeState(ERRORING);
					return;
				}
				_changeState(RESPONDING);
				file.close();
				return;
			}
		}
	}

	cout << "No index file found" << endl;
	_response.setStatusCode(HTTP_CODE_FORBIDDEN);
	_changeState(ERRORING);
}

void ClientHandler::_getResource() {
	const string path = _parsed_info.full_path;
	cout << "Getting resource for path: " << path << endl;

    struct stat info;
    bool fileExists = stat(path.c_str(), &info) == 0;
	if (!fileExists)
	{
		cout << "File does not exist or can't be accessed: " << path << endl;
		_response.setStatusCode(HTTP_CODE_NOT_FOUND);
		_changeState(ERRORING);
		return;
	}

	bool is_directory = path[path.size() - 1] == '/';
	if (is_directory) {
		cout << "Request for a directory" << endl;
		_getResourceDirectory();
		return;
	}

	bool isDir = S_ISDIR(info.st_mode);
	if (isDir) {
		cerr << "Found directory not a file" << endl;
		_response.setStatusCode(HTTP_CODE_NOT_FOUND);
		_changeState(ERRORING);
		return;
	}

	_response_info.fd_to_send = open(path.c_str(), O_RDONLY);
	if (_response_info.fd_to_send < 0) {
		cerr << "Error opening file descriptor for: " << path << endl;
		_response.setStatusCode(HTTP_CODE_FORBIDDEN);
		_changeState(ERRORING);
		return;
	}
	_changeState(RESPONDING);
}

void ClientHandler::_postResource() {
	cout << "Posting resource to path: " << _request->getPath() << " with content: " << _request->getBody() << endl;

	// _response_status_code = HTTP_CODE_CREATED;
}

void ClientHandler::_deleteResource() {
	const string path = _parsed_info.full_path;
	cout << "Deleting resource at path: " << path << endl;

	// if (remove(path.c_str()) != 0) {
	// 	cerr << "Error deleting file: " << path << endl;
	// 	_response_status_code = HTTP_CODE_FORBIDDEN;
	// 	return;
	// }

	// // resend page TOoO ANNAOYING?
	// _response = new HttpResponse(HTTP_CODE_NO_CONTENT);
	// _status = SENDING; // No generation needed
}

void ClientHandler::_process() {
	// PROCESS GET/POST/DELETE
	// int fd = _socket.getFd();
	// pollfd& pfd = _server->getPollFd(fd);
	const Location& matching_location = *_parsed_info.matching_location;

	if (matching_location.getRedirect().size() > 0) {
		cout << "Location has a redirect configured, redirecting to " << matching_location.getRedirect().begin()->second
		     << endl;
		_response.setStatusCode(matching_location.getRedirect().begin()->first);
		_response.addHeader("Location", matching_location.getRedirect().begin()->second);
		cout << "Redirecting to " << matching_location.getRedirect().begin()->second << endl;
		_changeState(RESPONDING); // No generation needed
		return;
	}

	const string path = _request->getPath();
	const string method = _request->getMethod();
	if (method == METHOD_GET) {
		cout << "Handling GET request for path: " << path << endl;
		_getResource();

	} else if (method == METHOD_POST) {
		cout << "Handling POST request for path: " << path << endl;
		_postResource();

	} else if (method == METHOD_DELETE) {
		cout << "Handling DELETE request for path: " << path << endl;
		_deleteResource();

	} else {
		_response.setStatusCode(HTTP_CODE_METHOD_NOT_ALLOWED);
		_changeState(ERRORING);
		return;
	}
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
	
	string chunk_to_send;
	ssize_t chunk_size;
	ssize_t bytes_sent;
	
	if (!_response_info.sent_headers) {
		chunk_to_send = _response.getHeadersString();
		_response_info.sent_headers = true;
		cout << "Sending headers to client on socket " << fd << endl;
		cout << chunk_to_send << endl;
		bytes_sent = send(fd, chunk_to_send.c_str(), chunk_to_send.size(), 0);
		if (bytes_sent < 0) {
			cout << "Error sending headers to client on socket " << fd << endl;
			_changeState(DONE);
			return;
		}
		return;
	}

	if (_response_info.fd_to_send == -1) {
		// Sending response body
		chunk_size = _response.getBodyChunk(chunk_to_send, _response_info.bytes_sent, _buffer_size);
		_response_info.bytes_sent += chunk_to_send.size();
		if (chunk_size == 0) {
			cout << "Finished sending response to client on socket " << fd << endl;
			_changeState(DONE);
			return;
		}
		if (chunk_size < 0) {
			cout << "Error getting response chunk for client on socket " << fd << endl;
			_changeState(DONE);
			return;
		}
		bytes_sent = send(fd, chunk_to_send.c_str(), chunk_to_send.size(), 0);

	} else {
		// Sending a file
		chunk_size = HttpUtils::chunkFile(_response_info.fd_to_send, _buffer_size, chunk_to_send);
		bytes_sent = send(fd, chunk_to_send.c_str(), chunk_to_send.size(), 0);

		if (bytes_sent < 0) {
			cout << "Error sending file to client on socket " << fd << endl;
			_changeState(DONE);
			return;
		}
		if (chunk_size == 0) {
			cout << "Finished sending file to client on socket " << fd << endl;
			_changeState(DONE);
			return;
		}
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
