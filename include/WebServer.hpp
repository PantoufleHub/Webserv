#pragma once

#include <string>
#include <vector>
#include <iostream>

class VirtualServer;

using namespace std;

class WebServer {
   private:
	vector<VirtualServer> _virtual_servers;

   public:
	WebServer(const string& config_file);
	~WebServer();

	void run();
	void display() const;
};