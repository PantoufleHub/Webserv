#pragma once

#include <string>
#include <map>
#include <iostream>
#include <sstream>

#include "Constants.hpp"

using namespace std;

class AHttpMessage {
   protected:
	string _http_version;
	map<string, string> _headers;
	string _body;
	virtual string _getStartLine() const = 0;
	virtual string _getStartLineFormatted() const = 0;
	string _getHeaders() const;

   public:
    virtual ~AHttpMessage() {}
	string toString() const;
	string toStringFormatted() const;
	void addHeader(string header, string value);
	void setBody(string body);
	void setHttpVersion(string http_version);

	const string& getHttpVersion() const;
	const map<string, string>& getHeaders() const;
	const string& getBody() const;
	size_t getBodyChunk(string &chunk, size_t pos, size_t length) const;
	const string getHeadersString() const;
};
