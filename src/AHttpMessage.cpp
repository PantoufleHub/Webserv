#include "AHttpMessage.hpp"

// AHttpMessage::AHttpMessage() {}

void AHttpMessage::addHeader(string header, string value) {
	cout << "Adding header: " << header << " with value: " << value << endl;
	this->_headers[header] = value;
	cout << "Header added" << endl;
}

void AHttpMessage::setBody(string body) {
	this->_body = body;

	// MAY NEED TO CHANGE WHEN IMPLEMENTING CHUNKED
	// IDK HOW WE GONNA IMPLEMENT CHUNKED >:O
	std::ostringstream ss;
	ss << body.length();
	string length = ss.str();

	this->addHeader(HEADER_CONTENT_LENGTH, length);
}

void AHttpMessage::setHttpVersion(string http_version) {
	this->_http_version = http_version;
}

string AHttpMessage::_getHeaders() const {
	ostringstream oss;

	map<string, string>::const_iterator iter = this->_headers.begin();
	while (iter != this->_headers.end()) {
		oss << (*iter).first << ": " << (*iter).second << "\r\n";
		iter++;
	}

	return oss.str();
}

string AHttpMessage::toString() const {
	ostringstream oss;

	oss << this->_getStartLine() << this->_getHeaders() << "\r\n" << this->_body;

	return oss.str();
}

string AHttpMessage::toStringFormatted() const {
	ostringstream oss;

	oss
	    // << "-- HTTP Message --\n"
	    << this->_getStartLineFormatted() << "\n-- Headers: \n"
	    << this->_getHeaders() << "\n-- Body:\n"
	    << this->_body << "\n"
	    << "------------------\n";

	return oss.str();
}

const string& AHttpMessage::getHttpVersion() const {
	return this->_http_version;
}

const map<string, string>& AHttpMessage::getHeaders() const {
	return this->_headers;
}

const string& AHttpMessage::getBody() const {
	return this->_body;
}
