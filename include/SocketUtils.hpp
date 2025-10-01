#pragma once

#include <arpa/inet.h>
#include <string>

using namespace std;

class SocketUtils {
   public:
	static sockaddr_in createSockaddr(string ip, int port);
};
