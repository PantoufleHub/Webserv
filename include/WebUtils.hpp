#pragma once

#include <arpa/inet.h>
#include <poll.h>
#include <string>

using namespace std;

class WebUtils {
   public:
	static sockaddr_in createSockaddr(string ip, int port);
	static bool canRead(pollfd &pfd);
	static bool canWrite(pollfd &pfd);
};
