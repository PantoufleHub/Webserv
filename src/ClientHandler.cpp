#include "ClientHandler.hpp"

void ClientHandler::_init_() {
	_state = CLIENT_READING;
	_buffer_size = DEFAULT_BUFFER_SIZE;
	_request = NULL;

	// Response info init
	_response_info.bytes_sent = 0;
	_response_info.sent_headers = false;
	_response_info.fd_to_send = -1;
	_response_info.content_type = TYPE_HTML;

	// Parsed info init
	_parsed_info.virtual_servers = NULL;
	_parsed_info.matching_server = NULL;
	_parsed_info.locations = NULL;
	_parsed_info.matching_location = NULL;
	_parsed_info.errors = NULL;
	_parsed_info.full_path = "";

	// Post info init
	_post_info.fd = -1;
	_post_info.parsed = false;

	// CGI info init
	_cgi_info.cgi_handler = NULL;
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
	cout << "~CH Destructor" << endl;
	cout << "Deleting CH for fd: " << _socket.getFd() << endl;
	if (_request) {
		cout << "Deleting request for fd: " << _socket.getFd() << endl;
		delete _request;
	}
	if (_response_info.fd_to_send != -1) {
		cout << "Closing file descriptor to send: " << _response_info.fd_to_send << endl;
		_server->removePollFd(_response_info.fd_to_send);
		close(_response_info.fd_to_send);
		_response_info.fd_to_send = -1;
	}
	if (_post_info.fd != -1) {
		cout << "Closing post info fd " << _post_info.fd << endl;
		_server->removePollFd(_post_info.fd);
		close(_post_info.fd);
		_post_info.fd = -1;
	}
	if (_cgi_info.cgi_handler) {
		cout << "Deleting CgiHandler" << endl;
		delete _cgi_info.cgi_handler;
	}
}

/// @brief Change the state of the ClientHandler
/// @param newState The state to change to
/// @param statusCode The response status code to change to (optional)
void ClientHandler::_changeState(ClientState newState, const int statusCode = 0) {
	_state = newState;
	int fd = _socket.getFd();
	pollfd &client_pfd = _server->getPollFd(fd);
	if (statusCode)
		_response.setStatusCode(statusCode);

	switch (newState) {
		case CLIENT_READING:
			client_pfd.events = POLLIN;
			break;
		case CLIENT_PARSING:
			client_pfd.events = 0;
			break;
		case CLIENT_PROCESSING:
			client_pfd.events = 0;
			break;
		case CLIENT_CGIING:
			client_pfd.events = 0;
			break;
		case CLIENT_ERRORING:
			client_pfd.events = 0;
			// Reset response
			_response.setBody(TYPE_HTML, "");
			break;
		case CLIENT_RESPONDING:
			cout	<< "Changing state to responding for client on socket " << fd << "\n"
					<< "Fd to send: " << _response_info.fd_to_send << endl;
			client_pfd.events = POLLOUT;
			_response.addHeader(HEADER_CONTENT_TYPE, _response_info.content_type);
	
			if (_response_info.fd_to_send == -1) {
				_response.addHeader(HEADER_CONTENT_LENGTH, StringUtils::sizetToString(_response.getBodySize()));
			}
			else {
				pollfd response_pfd;
				response_pfd.events = POLLIN;
				response_pfd.fd = _response_info.fd_to_send;
				_server->addPollFd(response_pfd);
				_response.addHeader(HEADER_TRANSFER_ENCODING, ENCODING_CHUNKED);
			}
			break;
		case CLIENT_DONE:
			client_pfd.events = 0;
			break;
	}
}

void ClientHandler::_checkRequestBuffer() {
	// CheckDataIn still needs to handle chunked transfer encoding
	int request_length = HttpRequestParser::checkDataIn(_request_buffer);
	if (request_length > 0) {
		cout << "Full request received on socket " << _socket.getFd() << endl;
		_request = HttpRequestParser::strToHttpRequest(_request_buffer.substr(0, request_length));
		_changeState(CLIENT_PARSING);
		Logger::logRequest(_request->toString(), _socket.getFd());
	} else if (request_length < 0) {
		cout << "Bad request on socket " << _socket.getFd() << endl;
		_changeState(CLIENT_DONE, HTTP_CODE_BAD_REQUEST); // Send error response? PL: BadRequest? Erroring?
	} else {
		// Incomplete request, keep reading
		if (_request_buffer.size() > this->_parsed_info.matching_server->getClientMaxBodySize()) {
			cout << "Request too large on socket " << _socket.getFd() << endl;
			_changeState(CLIENT_DONE, HTTP_CODE_PAYLOAD_TOO_LARGE); // Send error response? PL: Payload too large? Erroring?
		}
	}
}
//MAX_B_S = max(vs[])
void ClientHandler::_read() {
	int fd = _socket.getFd();
	pollfd &pfd = _server->getPollFd(fd);
	if (!WebUtils::canRead(pfd))
		return;

	char buffer[_buffer_size + 1];
	ssize_t bytes_received = recv(fd, buffer, _buffer_size, 0);

	if (bytes_received < 0) {
		cout << "Error reading from client " << fd << endl;
		_changeState(CLIENT_DONE); //error

	} else if (bytes_received == 0) {
		cout << "Client on socket " << fd << " disconnected" << endl;
		_changeState(CLIENT_DONE);
		
	} else {
		buffer[bytes_received] = '\0';
		string data_read(buffer, bytes_received);
		cout << "Received " << bytes_received << " bytes from client " << fd << endl;
		_request_buffer += data_read;
		if (_request_buffer.size() > MAX_REQUEST_LENGTH)
			_changeState(CLIENT_ERRORING, HTTP_CODE_PAYLOAD_TOO_LARGE);
		_checkRequestBuffer();
	}
}

void ClientHandler::_validateFirstLine() {
	const string method = _request->getMethod();
	const string path = _request->getPath();
	const string http_version = _request->getHttpVersion();

	if (method != METHOD_GET && method != METHOD_POST && method != METHOD_DELETE) {
		_changeState(CLIENT_ERRORING, HTTP_CODE_METHOD_NOT_ALLOWED);
		return;
	}

	// Prevent directory traversal attacks
	if (path.find("..") != string::npos || path.find("~") != string::npos) {
		_changeState(CLIENT_ERRORING, HTTP_CODE_BAD_REQUEST);
		return;
	}

	if (http_version != "HTTP/1.1") {
		_changeState(CLIENT_ERRORING, HTTP_CODE_HTTP_VERSION_NOT_SUPPORTED);
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

	cout << "Validating method '" << method << "' for location '" << matching_location.getNames()[0] << "'" << endl;
	
	if (!matching_location.checkMethod(method)) {
		cout << "Method '" << method << "' not allowed, setting 405" << endl;
		_changeState(CLIENT_ERRORING, HTTP_CODE_METHOD_NOT_ALLOWED);
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
	if (_state != CLIENT_PARSING)
		return;
	
	_parsed_info.virtual_servers = &_server->getVirtualServers();
	_parsed_info.entry_point = WebUtils::getSocketEntryPoint(_socket);
	_findVirtualServer();
	if (_state != CLIENT_PARSING)
		return;
	
	if (!_parsed_info.matching_server) {
		_changeState(CLIENT_ERRORING, HTTP_CODE_INTERNAL_SERVER_ERROR);
		return;
	}
	_parsed_info.errors = &_parsed_info.matching_server->getErrors();

	string path = _request->getPath();
	string full_path = _parsed_info.matching_server->getRoot() + (path[0] == '/' ? path.substr(1) : path);
	_parsed_info.full_path = full_path;

	_parsed_info.locations = &_parsed_info.matching_server->getLocations();
	if (_parsed_info.locations->empty()) {
		_changeState(CLIENT_ERRORING, HTTP_CODE_NOT_FOUND);
		return;
	}

	_findMatchingLocation();
	if (_parsed_info.matching_location == NULL) {
		_changeState(CLIENT_ERRORING, HTTP_CODE_NOT_FOUND);
		return;
	}

	_validateMatchingLocation();
	if (_state != CLIENT_PARSING)
		return;

	_changeState(CLIENT_PROCESSING);
}

void ClientHandler::_getResourceDirectory() {
	const Location& location = *_parsed_info.matching_location;
	const string path = _parsed_info.full_path;
	bool auto_index = location.getAutoIndex();
	
	if (auto_index) {
		cout << "Autoindex is enabled, generating page" << endl;
		HttpUtils::getAutoIndexPage(_response, location, path);
		_changeState(CLIENT_RESPONDING);
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
					cout << "Error opening file descriptor for: " << test_path << endl;
					_changeState(CLIENT_ERRORING, HTTP_CODE_FORBIDDEN);
					return;
				}
				_changeState(CLIENT_RESPONDING);
				file.close();
				return;
			}
		}
	}

	cout << "No index file found" << endl;
	_changeState(CLIENT_ERRORING, HTTP_CODE_FORBIDDEN);
}

void ClientHandler::_getResource() {
	const string path = _parsed_info.full_path;
	cout << "Getting resource for path: " << path << endl;

    struct stat info;
    bool fileExists = stat(path.c_str(), &info) == 0;
	if (!fileExists)
	{
		cout << "File does not exist or can't be accessed: " << path << endl;
		_changeState(CLIENT_ERRORING, HTTP_CODE_NOT_FOUND);
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
		cout << "Found directory not a file" << endl;
		_changeState(CLIENT_ERRORING, HTTP_CODE_NOT_FOUND);
		return;
	}

	_response_info.fd_to_send = open(path.c_str(), O_RDONLY);
	if (_response_info.fd_to_send < 0) {
		cout << "Error opening file descriptor for: " << path << endl;
		_changeState(CLIENT_ERRORING, HTTP_CODE_FORBIDDEN);
		return;
	}
	cout << "Opened fd: " << _response_info.fd_to_send << endl;
	_changeState(CLIENT_RESPONDING);
}

void ClientHandler::_postResource() {
	const string& body = _request->getBody();
	
	// NEEDS TO BE FIXED?
	if (!_post_info.parsed) {
		_post_info.parsed = true;
		const string& root = _parsed_info.matching_server->getRoot();
		const string& path = root.substr(0, root.size()-1) + _request->getPath();
		cout << "Posting resource to path: " << path << " with content: " << _request->getBody() << endl;
		string name = path; // Should not be the direct path! Do we care tho?

		struct stat statbuf;
		int stat_res = stat(name.c_str(), &statbuf);

		if (stat_res == 0) {
			cout << "File already exists" << endl;
			_changeState(CLIENT_ERRORING, HTTP_CODE_CONFLICT);
			return;
		}

		_post_info.fd = open(name.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0644);
		if (_post_info.fd < 0) {
			cout << "Error opening fd" << endl; 
			_changeState(CLIENT_ERRORING, HTTP_CODE_INTERNAL_SERVER_ERROR);
			return;
		}
		pollfd new_pfd;
		new_pfd.events = POLLOUT;
		new_pfd.fd = _post_info.fd;
		_server->addPollFd(new_pfd);
	} else {
		pollfd &pfd = _server->getPollFd(_post_info.fd);
		if (!WebUtils::canWrite(pfd)) {
			cout << "Unable to  write to fd " << _post_info.fd << endl;
			return;
		}
		static size_t pos = 0;
		ssize_t bytes_read;
	
		bytes_read = HttpUtils::write_data(_post_info.fd, body, pos, _buffer_size);
		if (bytes_read > 0) {
			cout << "Wrote " << bytes_read << " to file " << _post_info.fd << endl;
		}
		else if (bytes_read == 0) {
			cout << "Finished posting" << endl;
			_changeState(CLIENT_RESPONDING, HTTP_CODE_CREATED);
			return;
		}
		else if (bytes_read < 0) {
			_changeState(CLIENT_ERRORING, HTTP_CODE_INTERNAL_SERVER_ERROR);
			cout << "Error writing to file" << endl;
			return;
		}
	}

}

void ClientHandler::_deleteResource() {
	const string path = _parsed_info.full_path;
	cout << "Deleting resource at path: " << path << endl;

	if (remove(path.c_str()) != 0) {
		cout << "Error deleting file: " << path << endl;
		_changeState(CLIENT_ERRORING, HTTP_CODE_FORBIDDEN);
		return;
	}

	// resend page TOoO ANNAOYING?
	_changeState(CLIENT_RESPONDING, HTTP_CODE_NO_CONTENT);
	return; // useless? yes but whatever, i grew attached to it
}

void ClientHandler::_process() {
	// PROCESS GET/POST/DELETE
	// int fd = _socket.getFd();
	// pollfd& pfd = _server->getPollFd(fd);
	const Location& matching_location = *_parsed_info.matching_location;

	// redirection first
	if (matching_location.getRedirect().size() > 0) {
		cout	<< "Location has a redirect configured, redirecting to "
				<< matching_location.getRedirect().begin()->second << endl;
		_response.addHeader(HEADER_LOCATION, matching_location.getRedirect().begin()->second);
		_changeState(CLIENT_RESPONDING, matching_location.getRedirect().begin()->first); // No generation needed
		return;
	}

	const string path = _request->getPath();

	// script second
	// Not using cgi_index for now
	string cgistuff = _parsed_info.matching_location->getCgi();
	if (!cgistuff.empty()) {
		cout << "Found cgi_pass, changing to cgi state" << endl;
		_changeState(CLIENT_CGIING);
		return;
	}

	// ya mum third oooh ahaha lol
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
		_changeState(CLIENT_ERRORING, HTTP_CODE_METHOD_NOT_ALLOWED);
		return;
	}
}

void ClientHandler::_cgi() {
	int fd = _socket.getFd();

	if (!_cgi_info.cgi_handler) {
		cout << "Creating new CgiHandler for socket " << fd << endl;
		// BLEGH
		_cgi_info.cgi_handler = new CgiHandler(	_response,
												*_request,
												*_server,
												*_parsed_info.matching_server,
												*_parsed_info.matching_location,
												_socket);
		if (!_cgi_info.cgi_handler) {
			cout << "Error creating cgiHandler" << endl;
			_changeState(CLIENT_ERRORING, HTTP_CODE_INTERNAL_SERVER_ERROR);
			return;
		}
		//TEMP
		// delete _cgi_info.cgi_handler;
		// _cgi_info.cgi_handler = NULL;
		// _changeState(CLIENT_ERRORING, 567);
		// return;
	}

	if (_cgi_info.cgi_handler->getState() == CGI_ERROR) {
		_changeState(CLIENT_ERRORING, _cgi_info.cgi_handler->getErrorCode());
		return;
	}

	if (_cgi_info.cgi_handler->getState() == CGI_PROCESSING) {
		_cgi_info.cgi_handler->update();
		return;
	}

	if (_cgi_info.cgi_handler->getState() == CGI_FINISHED) {
		_changeState(CLIENT_RESPONDING);
		return;
	}


}

void ClientHandler::_error() {
	// HANDLE ERRORS
	int fd = _socket.getFd();
	cout << "Handling Error for request from client on socket " << fd << endl;

	HttpUtils::getErrorPage(_response, *_parsed_info.matching_location, _response.getStatusCode());

	_changeState(CLIENT_RESPONDING);
}

void ClientHandler::_respond() {
	int fd = _socket.getFd();
	int fd_to_send = _response_info.fd_to_send;
	pollfd &pfd = _server->getPollFd(fd); // ERROR HERE

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
			_changeState(CLIENT_DONE);
			return;
		}
	}
	if (fd_to_send == -1) {
		// Sending response body
		chunk_size = _response.getBodyChunk(chunk_to_send, _response_info.bytes_sent, _buffer_size);
		_response_info.bytes_sent += chunk_to_send.size();
		if (chunk_size == 0) {
			cout << "Finished sending response to client on socket " << fd << endl;
			_changeState(CLIENT_DONE);
			return;
		}
		if (chunk_size < 0) {
			cout << "Error getting response chunk for client on socket " << fd << endl;
			_changeState(CLIENT_DONE);
			return;
		}
		bytes_sent = send(fd, chunk_to_send.c_str(), chunk_to_send.size(), 0);

	} else {
		pollfd &pfd = _server->getPollFd(fd_to_send);
		if (!WebUtils::canRead(pfd)) {
			cout << "Not able to read from fd " << fd_to_send << endl;
			return;
		}
		// Sending a file
		chunk_size = HttpUtils::chunkFile(fd_to_send, _buffer_size, chunk_to_send);
		bytes_sent = send(fd, chunk_to_send.c_str(), chunk_to_send.size(), 0);

		if (bytes_sent < 0) {
			cout << "Error sending file to client on socket " << fd << endl;
			_changeState(CLIENT_DONE);
			return;
		}
		if (chunk_size == 0) {
			cout << "Finished sending file to client on socket " << fd << endl;
			_changeState(CLIENT_DONE);
			return;
		}
	}
}

void ClientHandler::update() {
	if (_state == CLIENT_READING)
		_read();
	if (_state == CLIENT_PARSING)
		_parse();
	if (_state == CLIENT_PROCESSING)
		_process();
	if (_state == CLIENT_CGIING)
		_cgi();
	if (_state == CLIENT_ERRORING)
		_error();
	if (_state == CLIENT_RESPONDING)
		_respond();
}

ClientState ClientHandler::getState() const {
	return _state;
}
