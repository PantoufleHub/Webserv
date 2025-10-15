#include "HttpRequest.hpp"

HttpRequest::HttpRequest() : _is_valid(true) {}

void HttpRequest::setMethod(string method) {
	this->_method = method;
}

void HttpRequest::setPath(string path) {
	this->_path = path;
}

/// @brief To be used when parsing a request
void HttpRequest::flagAsInvalid() {
	_is_valid = false;
}

void HttpRequest::flagAsValid() {
	_is_valid = true;
}

string HttpRequest::_getStartLine() const {
	ostringstream oss;

	oss << this->_method << " " << this->_path << " " << this->_http_version << "\r\n";

	return oss.str();
}

string HttpRequest::_getStartLineFormatted() const {
	ostringstream oss;

	oss << "-- HTTP Request --\n"
	    << "Method : " << this->_method << "\n"
	    << "Path   : " << this->_path << "\n"
	    << "Version: " << this->_http_version << endl;
	return oss.str();
}

const string& HttpRequest::getMethod() const {
	return this->_method;
}

const string& HttpRequest::getPath() const {
	return this->_path;
}

const bool& HttpRequest::isValid() const {
	return this->_is_valid;
}

const string HttpRequest::getHeaderValue(const string& header) const {
	map<string, string>::const_iterator iter = _headers.begin();
	while (iter != _headers.end()) {
		if (StringUtils::insCompare((*iter).first, header))
			return (*iter).second;
		iter++;
	}
	Logger::logError("getHeaderValue: header \"" + header + "\" not found ");
	return "";
}

bool HttpRequest::isHeaderSet(const string& header) const {
	map<string, string>::const_iterator iter = _headers.begin();
	while (iter != _headers.end()) {
		if (StringUtils::insCompare((*iter).first, header))
			return true;
		iter++;
	}
	return false;
}
