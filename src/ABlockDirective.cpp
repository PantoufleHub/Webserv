#include "ABlockDirective.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>

#include "ConfigParser.hpp"
#include "Constants.hpp"
#include "DirectiveKeys.hpp"
#include "Location.hpp"
#include "Logger.hpp"
#include "StringUtils.hpp"
#include "VirtualServer.hpp"

ABlockDirective::ABlockDirective() : _autoindex(false) {}

ABlockDirective::~ABlockDirective() {}

const std::map<int, std::string>& ABlockDirective::getErrors() const {
	return _errors;
}
const std::map<int, std::string>& ABlockDirective::getRedirect() const {
	return _redirect;
}
const std::vector<std::string>& ABlockDirective::getAllowedMethods() const {
	return _allowed_methods;
}
const std::vector<std::string>& ABlockDirective::getIndexes() const {
	return _indexes;
}
const std::vector<std::string>& ABlockDirective::getNames() const {
	return _names;
}
const std::vector<int>& ABlockDirective::getPorts() const {
	return _ports;
}
const std::string& ABlockDirective::getRoot() const {
	return _root;
}
const size_t& ABlockDirective::getClientMaxBodySize() const {
	return _client_max_body_size;
}
const std::string& ABlockDirective::getUploadStore() const {
	return _upload_store;
}
const bool& ABlockDirective::getAutoIndex() const {
	return _autoindex;
}

std::vector<std::string>& ABlockDirective::mutableIndexes() {
	return _indexes;
}
std::map<int, std::string>& ABlockDirective::mutableErrors() {
	return _errors;
}

void ABlockDirective::setBlockDirective(std::vector<std::string>::iterator& it) {
	const std::map<std::string, int> directives = getDirectivesMap();

	while (it->compare("}")) {
		if (!it->compare("{")) {
			++it;
			continue;
		}
		if (!it->compare(";")) {
			++it;
			continue;
		}
		std::map<std::string, int>::const_iterator dirIt = directives.find(*it);
		if (dirIt == directives.end()) {
			const std::string error = "Unknown directive '" + *it + "'";
			throw ConfigParser::ParsingException(error.c_str());
		}
		++it;
		SetDirectives(dirIt->second, it, *this);
		if (!this->getClientMaxBodySize())
			this->setClientMaxBodySize();
	}
}

void ABlockDirective::addRootPrefixToErrors() {
	const std::string root = getRoot();
	for (std::map<int, std::string>::iterator it = _errors.begin(); it != _errors.end(); ++it) {
		std::string relativePath = it->second;
		if (!root.empty() && root[root.size() - 1] != '/' && !relativePath.empty() && relativePath[0] != '/') {
			relativePath.insert(0, 1, '/');
		}
		_errors[it->first] = root + relativePath;
	}
}

void ABlockDirective::addRootPrefixToVectorOfString(std::vector<std::string>& path) {
	const std::string root = getRoot();
	for (std::size_t i = 0; i < path.size(); ++i) {
		std::string relativePath = path[i];
		if (!root.empty() && root[root.size() - 1] != '/' && (relativePath.empty() || relativePath[0] != '/')) {
			relativePath.insert(0, 1, '/');
		}
		path[i] = root + relativePath;
	}
}

void ABlockDirective::addRootSuffix() {
	if (!_root.empty() && _root[_root.size() - 1] != '/') {
		_root.push_back('/');
	}
}

void ABlockDirective::setAllowedMethods(std::vector<std::string>::iterator& it) {
	for (; it->compare(";"); ++it) {
		if (*it == METHOD_GET || *it == METHOD_POST || *it == METHOD_DELETE) {
			_allowed_methods.push_back(*it);
		} else if (*it == METHOD_NONE) {
			if (_allowed_methods.empty()) {
				_allowed_methods.push_back(*it);
			} else {
				throw ConfigParser::ParsingException("none should be used as single directive");
			}
			++it;
			break;
		} else {
			const std::string error = *it + " Method not allowed";
			throw ConfigParser::ParsingException(error.c_str());
		}
	}
}

void ABlockDirective::setIndexes(std::vector<std::string>::iterator& it) {
	for (; it->compare(";"); ++it) {
		_indexes.push_back(*it);
	}
}

void ABlockDirective::setNames(std::vector<std::string>::iterator& it) {
	std::string eol = ";";
	bool isLocation = false;
	if (dynamic_cast<Location*>(this)) {
		_names.push_back(*it);
		eol = "{";
		isLocation = true;
	}
	for (; it->compare(eol); ++it) {
		if (!isLocation) {
			_names.push_back(*it);
		}
	}
}

void ABlockDirective::setPorts(std::vector<std::string>::iterator& it) {
	for (; it->compare(";"); ++it) {
		_ports.push_back(std::atoi(it->c_str()));
	}
}

void ABlockDirective::setErrors(std::vector<std::string>::iterator& it) {
	while (it->compare(";")) {
		const int code = std::atoi(it->c_str());
		++it;
		if (!it->compare(";")) {
			break;
		}
		_errors[code] = *it;
		++it;
	}
}

void ABlockDirective::setClientMaxBodySize(std::vector<std::string>::iterator& it) {
	char*	cerror;
	size_t maxBodySize = std::strtol(it->c_str(), &cerror, 10);
	string	error = cerror;
	if (!maxBodySize || !error.empty()) {
		const std::string error = "Client max body size must be an integer (size_t) value '" + *it + "' Too large or bad format";
		throw ConfigParser::ParsingException(error.c_str());
	}
	++it;
}

void ABlockDirective::setClientMaxBodySize() {
	_client_max_body_size = MAX_REQUEST_LENGTH;
	cout << "Missing Content length directive, setting the Max body size to the 1Mo default value" << endl;
}

void ABlockDirective::setUploadStore(std::vector<std::string>::iterator& it) {
	std::ifstream dir(it->c_str());
	if (dir.good()) {
		_upload_store = *it;
	} else {
		const std::string error = "Wrong path for Upload Store: " + *it;
		Logger::logError(error + " | Falling back to default upload store");
		_upload_store = DEFAULT_UPLOAD_STORE;
	}
	++it;
}

void ABlockDirective::setAutoIndex(std::vector<std::string>::iterator& it) {
	if (!it->compare("on")) {
		_autoindex = true;
	} else if (!it->compare("off")) {
		_autoindex = false;
	} else {
		Logger::logError("Wrong syntax for autoindex, expected on/off: " + *it);
		_autoindex = false;
	}
	++it;
}

void ABlockDirective::setRedirect(std::vector<std::string>::iterator& it) {
	while (it->compare(";")) {
		const int code = std::atoi(it->c_str());
		++it;
		if (!it->compare(";")) {
			break;
		}
		_redirect[code] = *it;
		++it;
	}
}

void ABlockDirective::setRoot(std::vector<std::string>::iterator& it) {
	_root = *it;
	++it;
}
