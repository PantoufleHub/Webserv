#include <WebServer.hpp>

WebServer::WebServer(const string& config_file) {
	ConfigParser parsedFile(config_file);
	parsedFile.tokeniseConfigFile();

	_virtual_servers = parsedFile.parseTokens(parsedFile.getTokens());
}

WebServer::~WebServer() {
	
}


/// @param listening_socket Socket the client connected to
/// @return The new client socket fd
int WebServer::_openClientSocket(int listening_socket) {
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
	// Change the events !
	new_client_pollfd.events |= POLLOUT;
	_fds.push_back(new_client_pollfd);

	Logger::logConnection(new_client_addr, new_client_pollfd.fd);
	cout << "New client on socket: " << new_client_pollfd.fd << endl;

	return new_client_pollfd.fd;
}

void WebServer::run() {
	string ip = "0.0.0.0";
	int port = 8080;
	int backlog = 10;
	int poll_timeout = 1000;

	int server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	ASSERT(server_socket_fd != -1);

	sockaddr_in server_addr = SocketUtils::createSockaddr(ip, port);

	int opt = 1;
	ASSERT(setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != -1);

	ASSERT(::bind(server_socket_fd, (sockaddr*)&server_addr, (socklen_t)sizeof(sockaddr_in)) != -1);

	ASSERT(listen(server_socket_fd, backlog) != -1);
	cout << "Server listening on port: " << port << endl;

	fcntl(server_socket_fd, fcntl(server_socket_fd, F_GETFL, 0) | O_NONBLOCK);

	pollfd server_pollfd;
	server_pollfd.events = POLLIN;
	server_pollfd.fd = server_socket_fd;

	_fds.push_back(server_pollfd);

	while (1) {
		cout << poll (&_fds[0], _fds.size(), poll_timeout) << " pollfds updated" << endl;

		for (size_t fd_index = 0; fd_index < _fds.size(); fd_index++) {
			if (_fds[fd_index].revents & POLLIN) {
				if (fd_index == 0)
					_openClientSocket(_fds[fd_index].fd);
			}
			if (_fds[fd_index].revents & POLLOUT) {
				char buf[] = "HTTP/1.1 200 KK\r\nConnection: close\r\n\r\ncon";
				string bufstr(buf);
				send(_fds[fd_index].fd, buf, bufstr.size(), 0);
				close(_fds[fd_index].fd);
				_fds.erase(_fds.begin() + fd_index);
				fd_index--;
			}
		}
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
