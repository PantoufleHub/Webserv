#include "Location.hpp"

#include "StringUtils.hpp"
#include "VirtualServer.hpp"

Location::Location() : ABlockDirective() {}
Location::~Location() {}

const std::string& Location::getCgi() const {
	return _cgi_pass;
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
	_cgi_pass = *it;
	++it;
}

bool Location::checkMethod(const std::string& method) const {
	return StringUtils::strInStrs(method, _allowed_methods);
}
