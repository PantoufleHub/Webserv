#pragma once

#include "AHttpMessage.hpp"
#include "StringUtils.hpp"
#include "HttpUtils.hpp"

class HttpResponse : public AHttpMessage {
   private:
	int _status_code;
	string _getStartLine() const;
	string _getStartLineFormatted() const;
	// const string _getHttpStatusMessage(int status_code) const;
   public:
	HttpResponse();
	HttpResponse(int status_code);
	const int& getStatusCode() const;

	void addBody(string content_type, string text);
	void setBody(string content_type, string body);
	void setStatusCode(int status_code);
};
