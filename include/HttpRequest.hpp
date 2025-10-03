#pragma once

#include "AHttpMessage.hpp"
#include "StringUtils.hpp"
#include "Logger.hpp"

class HttpRequest : public AHttpMessage {
   private:
	bool _is_valid;
	string _method;
	string _path;
	string _getStartLine() const;
	string _getStartLineFormatted() const;

   public:
	HttpRequest();

	const string& getMethod() const;
	const string& getPath() const;
	const bool& isValid() const;
	const string getHeaderValue(const string& header) const;

	void setMethod(string method);
	void setPath(string path);

	bool isHeaderSet(const string& header) const;
	void flagAsInvalid();
	void flagAsValid();
};
