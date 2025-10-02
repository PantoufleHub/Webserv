#include <WebServer.hpp>

WebServer::WebServer(const string& config_file) {
	ConfigParser parsedFile(config_file);
	parsedFile.tokeniseConfigFile();

	_virtual_servers = parsedFile.parseTokens(parsedFile.getTokens());
}

WebServer::~WebServer() {
	
}

void WebServer::_openListeningSocket(string ip, int port) {
	int server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	ASSERT(server_socket_fd != -1);

	sockaddr_in server_addr = WebUtils::createSockaddr(ip, port);

	int opt = 1;
	ASSERT(setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != -1);

	ASSERT(::bind(server_socket_fd, (sockaddr*)&server_addr, (socklen_t)sizeof(sockaddr_in)) != -1);

	ASSERT(listen(server_socket_fd, DEFAULT_SOCKET_BACKLOG) != -1);
	cout << "Server listening on port: " << port << " Socket " << server_socket_fd << endl;

	fcntl(server_socket_fd, fcntl(server_socket_fd, F_GETFL, 0) | O_NONBLOCK);

	pollfd server_pollfd;
	server_pollfd.events = POLLIN;
	server_pollfd.fd = server_socket_fd;

	_listening_sockets.push_back(Socket(server_socket_fd));
	_pollfds.push_back(server_pollfd);
}

/// @brief Opens a listening socket for each VirtualServer ip:port entry point combination
void WebServer::_openAllServerSockets() {
	vector<EntryPoint> opened_entries;

	for (size_t server_i = 0; server_i < _virtual_servers.size(); server_i++) {
		VirtualServer server = _virtual_servers[server_i];
		vector<EntryPoint> server_entries = server.getEntryPoints();
		for (size_t entry_i = 0; entry_i < server_entries.size(); entry_i++) {
			EntryPoint ep = server_entries[entry_i];
			if (!EpInEps(ep, opened_entries)) {
				opened_entries.push_back(ep);
				cout << "Opening socket on entry point: " << ep.ip << ":" << ep.port << endl;
				_openListeningSocket(ep.ip, ep.port);
			}
		}
	}
}

void WebServer::_updateListeningSockets() {
	for (size_t index = 0; index < _listening_sockets.size(); index++) {
		int listening_fd = _listening_sockets[index].getFd();

		// Look for the corresponding pollfd
		for (size_t i = 0; i < _pollfds.size(); i++) {
			if (_pollfds[i].fd == listening_fd) {
				pollfd& poll_fd = _pollfds[i];

				// If the listening socket is ready to read, accept a new connection
				if (WebUtils::canRead(poll_fd)) {
					_openClientSocket(poll_fd.fd);
				}
				break;
			}
		}
	}
}

void WebServer::_openClientSocket(int listening_socket) {
	sockaddr_in new_client_addr;
	socklen_t new_client_addr_len = sizeof(new_client_addr);
	int new_client_socket_fd;

	cout << "Accepting client from socket " << listening_socket  << endl;
	new_client_socket_fd = accept(listening_socket, (sockaddr*)&new_client_addr, &new_client_addr_len);
	ASSERT(new_client_socket_fd != -1);
	cout << "client connected: " << inet_ntoa(new_client_addr.sin_addr) << ":" << ntohs(new_client_addr.sin_port)
	     << endl;

	fcntl(new_client_socket_fd, fcntl(new_client_socket_fd, F_GETFL, 0) | O_NONBLOCK);

	struct pollfd new_client_pollfd;
	new_client_pollfd.fd = new_client_socket_fd;
	// Change the events !?
	new_client_pollfd.events = POLLIN;
	_clients[new_client_socket_fd] = ClientHandler(Socket(new_client_socket_fd), this);
	_pollfds.push_back(new_client_pollfd);

	Logger::logConnection(new_client_addr, new_client_pollfd.fd);
	cout << "New client on socket: " << new_client_pollfd.fd << endl;
}

void WebServer::_updateClientSockets() {
	// Iterate after the listening sockets TEMP!
	for (size_t index = _listening_sockets.size(); index < _pollfds.size(); index++) {
		pollfd& pfd = _pollfds[index];
		if (WebUtils::canRead(pfd)) {
			char buffer[1024];
			int bytes_received = recv(pfd.fd, buffer, sizeof(buffer) - 1, 0);
			if (bytes_received <= 0) {
				// Connection closed or error
				cout << "Client disconnected: " << pfd.fd << endl;
				Logger::logDisconnection(pfd.fd);
				close(pfd.fd);
				_pollfds.erase(_pollfds.begin() + index);
				index--;
			} else {
				buffer[bytes_received] = '\0'; // Null-terminate the received data
				string request(buffer);
				Logger::logRequest(request, pfd.fd);
				pfd.events = POLLOUT;
			}
		}
		if (WebUtils::canWrite(pfd)) {
			char buf[] = "HTTP/1.1 200 KK\r\nConnection: close\r\n\r\ncon";
			string bufstr(buf);
			send(pfd.fd, buf, bufstr.size(), 0);
			Logger::logResponse(bufstr, pfd.fd);
			close(pfd.fd);
			_pollfds.erase(_pollfds.begin() + index);
			// CREATE CLIENT ERASE FUNCTION
		}
	}
}

void WebServer::run() {
	_openAllServerSockets();
	int poll_timeout = 1000;

	while (1) {
		cout << poll (&_pollfds[0], _pollfds.size(), poll_timeout) << " pollfds updated" << endl;

		_updateListeningSockets();
		_updateClientSockets();

		// GARBAGE? clear clients
	}
}

void WebServer::display() const {
	for (size_t i = 0; i < _virtual_servers.size(); i++) {
		cout << "Server: " << _virtual_servers[i].getNames()[0] << " ; Root: " << _virtual_servers[i].getRoot() << endl;
		map<int, string> errors = _virtual_servers[i].getErrors();
		vector<string> indexes = _virtual_servers[i].getIndexes();
		vector<string> allowed_methods = _virtual_servers[i].getAllowedMethods();
		vector<Location> locations = _virtual_servers[i].getLocations();
		vector<EntryPoint> eP = _virtual_servers[i].getEntryPoints();
		cout << "\nErrors: " << endl;
		for (map<int, string>::iterator it = errors.begin(); it != errors.end(); it++) {
			cout << "Error " << it->first << "->" << it->second << endl;
		}
		cout << "\nIndexes: " << endl;
		for (vector<string>::iterator it = indexes.begin(); it != indexes.end(); it++) {
			cout << *it << endl;
		}
		cout << "\nAllowed_methods: " << endl;
		for (vector<string>::iterator it = allowed_methods.begin(); it != allowed_methods.end(); it++) {
			cout << *it << endl;
		}
		for (size_t i = 0; i < locations.size(); i++) {
			cout << "\nLocations" << "[" << i << "]: " << flush;
			map<string, vector<string> > cgi = locations[i].getCgi();
			map<int, string> redirect = locations[i].getRedirect();
			if (cgi.empty()) {
				cout << locations[i].getNames()[0] << "\nRoot: " << locations[i].getRoot()
				     << "\nRedirect: " << locations[i].getNames()[0] << endl;
				for (map<int, string>::iterator itRedir = redirect.begin(); itRedir != redirect.end(); itRedir++) {
					cout << "Key: " << itRedir->first << " Values: " << itRedir->second << flush;
				}
				cout << "\nAutoIndex: " << locations[i].getAutoIndex()
				     << "\nUpload Store: " << locations[i].getUploadStore() << "\nAllowed Methods: " << endl;
				for (size_t it = 0; it < locations[i].getAllowedMethods().size(); it++)
					cout << locations[i].getAllowedMethods()[it] << endl;
			} else {
				cout << "Cgi: " << locations[i].getNames()[0] << endl;
				for (map<string, vector<string> >::iterator itCgi = cgi.begin(); itCgi != cgi.end(); itCgi++) {
					cout << "Key: " << itCgi->first << " Values: " << flush;
					for (size_t i = 0; i < itCgi->second.size(); i++)
						cout << itCgi->second[i] << "; " << flush;
					cout << endl;
				}
			}
		}
		cout << "\nEntryPoints:\n" << flush;
		for (size_t i = 0; i < eP.size(); i++) {
			cout << "EntryPoint " << i << ":\nPort: " << eP[i].port << " IP: " << eP[i].ip << endl;
		}
		cout << endl;
	}
}
