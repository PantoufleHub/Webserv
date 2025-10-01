#include <iostream>
#include <vector>

#include "ConfigParser.hpp"
#include "WebServer.hpp"

int main(int argc, char** argv) {
	if (argc > 2) {
		std::cerr << "Usage: " << argv[0] << " [config_file]" << std::endl;
		return 1;
	}

	std::string configPath;
	if (argc == 2) {
		configPath = argv[1];
	} else {
		configPath = "misc/config_files/default_config.conf";
	}

	try {
		WebServer Server(configPath);
		Server.display();
		Server.run();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}
