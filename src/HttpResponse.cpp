#include "HttpResponse.hpp"

HttpResponse::HttpResponse() : _status_code(200) {
	this->_http_version = HTTP_VERSION;
	// Since our server doesn't support persistent connections
	this->addHeader(HEADER_CONNECTION, "close");
	this->addHeader(HEADER_CONTENT_LENGTH, "0");
}

HttpResponse::HttpResponse(int status_code) : _status_code(status_code) {
	this->_http_version = HTTP_VERSION;
	// Since our server doesn't support persistent connections
	this->addHeader(HEADER_CONNECTION, "close");
	this->addHeader(HEADER_CONTENT_LENGTH, "0");
}

void HttpResponse::setBody(string content_type, string body) {
	this->addHeader(HEADER_CONTENT_TYPE, content_type);
	this->addHeader(HEADER_CONTENT_LENGTH, StringUtils::sizetToString(body.length()));
	AHttpMessage::setBody(body);
}

void HttpResponse::addBody(string content_type, string text) {
	setBody(content_type, _body + text);
}

string HttpResponse::_getStartLine() const {
	ostringstream oss;

	oss << this->_http_version << " " << this->_status_code << " "
	    << HttpUtils::getHttpStatusMessage(this->_status_code) << "\r\n";

	return oss.str();
}

string HttpResponse::_getStartLineFormatted() const {
	ostringstream oss;

	oss << "-- Http Response --\n"
	    << "Status : " << this->_status_code << "\n"
	    << "Message: " << HttpUtils::getHttpStatusMessage(_status_code) << "\n"
	    << "Version: " << this->_http_version << endl;
	return oss.str();
}

const int& HttpResponse::getStatusCode() const {
	return this->_status_code;
}

void HttpResponse::setStatusCode(int status_code) {
	_status_code = status_code;
}
