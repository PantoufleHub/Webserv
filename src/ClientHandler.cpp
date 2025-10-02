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

}

ClientHandler::~ClientHandler() {}

void ClientHandler::update() {
	// TO BE IMPLEMENTED
}