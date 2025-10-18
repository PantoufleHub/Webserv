#include "ConfigParser.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <arpa/inet.h>  // Add this include

#include "ABlockDirective.hpp"
#include "Constants.hpp"
#include "Logger.hpp"

ConfigParser::ConfigParser(const std::string& filePath) : _filePath(filePath) {}

static bool isValidIP(const string& ip) {
	if (ip == "0.0.0.0")
		return true;
	
	struct sockaddr_in sa;
	int result = inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr));

	if (result == 1)
		return true;
	
	if (ip == "localhost")
		return true;
	
	return false;
}

static void checkConflictingEntryPoints(const vector<VirtualServer>& servers) {
	map<string, vector<string> > ep_to_servers;
	
	for (size_t i = 0; i < servers.size(); i++) {
		const VirtualServer& server = servers[i];
		const vector<EntryPoint>& eps = server.getEntryPoints();
		const vector<string>& names = server.getNames();

		string current_server_name = names.empty() ? "(unnamed)" : names[0];

		for (size_t j = 0; j < eps.size(); j++) {
			ostringstream ep_key;
			ep_key << eps[j].ip << ":" << eps[j].port;
			string key = ep_key.str();

			ep_to_servers[key].push_back(current_server_name);
		}
	}

	for (map<string, vector<string> >::iterator it = ep_to_servers.begin();
		it != ep_to_servers.end(); ++it) {
		if (it->second.size() > 1) {
			ostringstream warning;
			warning << "Multiple servers listening on " << it->first << ": ";
			for (size_t i = 0; i < it->second.size(); i++) {
					warning << it->second[i];
					if (i < it->second.size() - 1)
						warning << ", ";
			}
			warning << "\nUpdate your config file accordingly";
			throw ConfigParser::ParsingException(warning.str().c_str());
		}
	}
}

static bool isValidServer(VirtualServer& server) {
	if (server.getEntryPoints().empty()) {
		Logger::logError("Server block missing listen directive");
		return false;
	}

	if (server.getRoot().empty()) {
		Logger::logError("Server block missing root directive");
		return false;
	}

	std::ifstream dir(server.getRoot().c_str());
	const bool exists = dir.good();
	dir.close();

	if (!exists) {
		Logger::logError("Server block root directive isn't a directory");
		return false;
	}

	server.addRootSuffix();
	server.addRootPrefixToErrors();
	return true;
}

static void validateAndPushBack(VirtualServer& candidate, std::vector<VirtualServer>& servers) {
	if (isValidServer(candidate)) {
		servers.push_back(candidate);
	} else {
		throw ConfigParser::ParsingException("Invalid server block detected");
	}
}

static std::vector<std::string> splitToken(std::string& token) {
	std::vector<std::string> tokensplitted;
	std::string buffer;

	for (std::string::iterator it = token.begin(); it != token.end(); ++it) {
		if (*it == '{' || *it == '}' || *it == ';') {
			if (!buffer.empty()) {
				tokensplitted.push_back(buffer);
				buffer.clear();
			}
			tokensplitted.push_back(std::string(1, *it));
		} else {
			buffer += *it;
		}
	}

	if (!buffer.empty()) {
		tokensplitted.push_back(buffer);
	}

	return tokensplitted;
}

void fillEntryPoint(std::vector<std::string>::iterator& it, EntryPoint& entryPoint, VirtualServer& block) {
	for (; it->compare(";"); ++it) {
		size_t colon_pos = it->find(':');
		if (colon_pos == string::npos)
			throw ConfigParser::ParsingException("Invalid listen format. Expected IP:PORT (e.g. 127.0.0.1:8080)");
		
		entryPoint.ip = it->substr(0, colon_pos);
		string port_str = it->substr(colon_pos + 1);

		if (!isValidIP(entryPoint.ip)) {
			string error = "Invalid IP address: " + entryPoint.ip;
			throw ConfigParser::ParsingException(error.c_str());
		}
		
		char* end;
		long port_long = strtol(port_str.c_str(), &end, 10);

		if (*end != '\0') {
			string error = "Port must be a number: " + port_str;
			throw ConfigParser::ParsingException(error.c_str());
		}

		if (port_long < 1 || port_long > 65535) {
			ostringstream error;
			error << "Port number out of range (1-65535): " << port_long;
			throw ConfigParser::ParsingException(error.str().c_str());
		}

		entryPoint.port = static_cast<int>(port_long);
		block.addEntryPoint(entryPoint);
	}
}

std::vector<VirtualServer> ConfigParser::parseTokens(std::vector<std::string> tokens) {
	std::vector<VirtualServer> servers;
	std::string error;

	for (std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
		if (!it->compare("server")) {
			VirtualServer candidate;
			++it;
			candidate.setBlockDirective(it);
			validateAndPushBack(candidate, servers);
			if (it->compare("}")) {
				error = "Couldn't setup virtual server, verify config file " + _filePath;
			}
		} else {
			error = "Couldn't setup virtual server, verify config file " + _filePath;
		}
	}

	if (!error.empty()) {
		throw ConfigParser::ParsingException(error.c_str());
	}

	checkConflictingEntryPoints(servers);

	return servers;
}

void ConfigParser::tokeniseConfigFile() {
	std::ifstream file(_filePath.c_str());
	if (!file.is_open()) {
		const std::string error = "Error opening the config file " + _filePath;
		throw ConfigParser::ParsingException(error.c_str());
	}

	std::vector<std::string> lines;
	std::string line;

	while (std::getline(file, line)) {
		const std::size_t comment = line.find('#');
		if (comment != std::string::npos) {
			line = line.substr(0, comment);
		}
		if (line.find_first_not_of(" \t\n") == std::string::npos) {
			continue;
		}
		lines.push_back(line);
	}

	for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it) {
		std::istringstream iss(*it);
		std::string token;
		while (iss >> token) {
			std::vector<std::string> tokensplitted = splitToken(token);
			_tokens.insert(_tokens.end(), tokensplitted.begin(), tokensplitted.end());
		}
	}

	file.close();
}

std::vector<std::string> ConfigParser::getTokens() const {
	return _tokens;
}

ConfigParser::ParsingException::ParsingException(const char* msg) : _message(msg) {}
ConfigParser::ParsingException::~ParsingException() throw() {}
const char* ConfigParser::ParsingException::what() const throw() {
	return _message.c_str();
}
