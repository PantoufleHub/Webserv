#include "Location.hpp"

#include "StringUtils.hpp"
#include "VirtualServer.hpp"

Location::Location() : ABlockDirective() {}
Location::~Location() {}

const std::map<std::string, std::vector<std::string> >& Location::getCgi() const {
	return _cgi_directives;
}

void Location::inheritFromParent(const VirtualServer& parent) {
	if (_allowed_methods.empty()) {
		_allowed_methods = parent.getAllowedMethods();
	}
	if (_root.empty()) {
		_root = parent.getRoot();
	}
	if (_errors.empty()) {
		_errors = parent.getErrors();
	}
	if (_indexes.empty()) {
		_indexes = parent.getIndexes();
	}
}

void Location::setCgi(std::vector<std::string>::iterator& it) {
	--it;
	std::string key = *it;
	++it;
	std::vector<std::string> value;
	for (; *it != ";"; ++it) {
		value.push_back(*it);
	}
	_cgi_directives.insert(std::make_pair(key, value));
}

bool Location::checkMethod(const std::string& method) const {
	return StringUtils::strInStrs(method, _allowed_methods);
}
