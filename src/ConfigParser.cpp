#include "ConfigParser.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

#include "ABlockDirective.hpp"
#include "Constants.hpp"
#include "Logger.hpp"

ConfigParser::ConfigParser(const std::string& filePath) : _filePath(filePath) {}

void fillEntryPoint(std::vector<std::string>::iterator& it, EntryPoint& entryPoint, VirtualServer& block) {
	for (; it->compare(";"); ++it) {
		const std::size_t colonPos = it->find(':');
		entryPoint.ip = it->substr(0, colonPos);
		entryPoint.port = std::atoi(it->substr(colonPos + 1).c_str());
		entryPoint.backlog = 10;
		block.addEntryPoint(entryPoint);
	}
}

namespace {

bool isValidServer(VirtualServer& server) {
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

void validateAndPushBack(VirtualServer& candidate, std::vector<VirtualServer>& servers) {
	if (isValidServer(candidate)) {
		servers.push_back(candidate);
	} else {
		throw ConfigParser::ParsingException("Invalid server block detected");
	}
}

std::vector<std::string> splitToken(std::string& token) {
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

}  // namespace

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
