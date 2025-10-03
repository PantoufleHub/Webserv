#include "HttpRequestParser.hpp"

string HttpRequestParser::_getLine(istringstream& iss) {
	string ret = "";

	while (!iss.eof()) {
		char c = iss.get();
		ret += c;
		// cout << "Char: " << c << endl;
		if (c == '\n')
			break;
	}

	return ret;
}

bool HttpRequestParser::_parseCLRF(string& str) {
	// cout << "ParseCLRF: " << str << "|" <<  endl;
	size_t len = str.length();
	if (len < 2)
		return 0;
	else {
		if (str[len - 2] != '\r' || str[len - 1] != '\n')
			return 0;
	}
	str.erase(str.size() - 2);
	return 1;
}

bool HttpRequestParser::_parseFirstLine(HttpRequest& request, istringstream& iss) {
	string first_line = _getLine(iss);
	istringstream line_stream(first_line);
	if (_parseCLRF(first_line)) {

		if (StringUtils::nbCharsInStr(' ', first_line) != 2)
			return 0;

		string method, path, Http_version;
		if (!(line_stream >> method >> path >> Http_version))
			return 0;

		request.setMethod(method);
		request.setPath(path);
		request.setHttpVersion(Http_version);
		return 1;
	}
	return 0;
}

bool HttpRequestParser::_parseHeaders(HttpRequest& request, istringstream& iss) {
	string line;
	string header;
	string value;
	size_t separator;
	while (true) {
		line = _getLine(iss);

		if (line == "\r\n")
			break;
		if (!_parseCLRF(line))
			return 0;
		separator = line.find(":");
		if (separator == string::npos)
			return 0;
		// Accept ":" or just ": " ?
		header = line.substr(0, separator);
		value = line.substr(separator + 2, string::npos);
		if (header == "")
			return 0;
		request.addHeader(header, value);
	}
	return 1;
}

bool HttpRequestParser::_parseBody(HttpRequest& request, istringstream& iss) {
	string body;
	getline(iss, body);
	request.setBody(body);
	return 1;
}

bool HttpRequestParser::_validateRequest(HttpRequest& ret) {
	if (!ret.isHeaderSet("host"))
		return false;
	if (ret.isHeaderSet("content-length") &&
	    (strtold(ret.getHeaderValue("content-length").c_str(), NULL) != ret.getBody().length()))
		return false;

	// CHUNKED REQUESTS

	return true;
}

HttpRequest HttpRequestParser::strToHttpRequest(string str) {
	HttpRequest ret;
	istringstream iss(str);

	ret.flagAsInvalid();

	if (!_parseFirstLine(ret, iss))
		return ret;

	if (!_parseHeaders(ret, iss))
		return ret;

	if (!_parseBody(ret, iss))
		return ret;

	if (!_validateRequest(ret))
		return ret;

	ret.flagAsValid();
	return ret;
}

static int getContentLength(const string& dataIn) {
	const string	content_length_str = "Content-Length: ";
	size_t			beginning = dataIn.find(content_length_str);
	size_t			end = dataIn.find("\r\n", beginning);
	if (end == string::npos || beginning == string::npos)
		return -1;
	string header_line = dataIn.substr(beginning, end - beginning);
	if (header_line.empty())
		return -1;

	stringstream ss(header_line.substr(content_length_str.size()));
	int content_length;
	if (ss >> content_length && content_length >= 0) {
		return content_length;
	}
	return -1;
}

int HttpRequestParser::checkDataIn(const string& _data_in) {
	size_t request_end = _data_in.find("\r\n\r\n");
	if (request_end == string::npos) {
		return 0;
	}

	size_t first_line_end = _data_in.find("\r\n");
	string first_line = _data_in.substr(0, first_line_end);
	size_t space1 = first_line.find(' ');
	size_t space2 = first_line.find(' ', space1 + 1);

	if (space1 == string::npos || space2 == string::npos || space2 <= space1 + 1) {
		Logger::logError("Bad request line format");
		return -1;
	}

	string method = first_line.substr(0, space1);
	string path = first_line.substr(space1 + 1, space2 - space1 - 1);
	string version = first_line.substr(space2 + 1);

	if (method != "GET" && method != "POST" && method != "DELETE") {
		Logger::logError("Bad method: " + method);
		return -1;
	}

	if (path.empty() || path[0] != '/') {
		Logger::logError("Bad path: " + path);
		return -1;
	}

	if (method == "POST") {
		int content_length = getContentLength(_data_in);
		if (content_length == -1) {
			Logger::logError("POST without Content-Length, assuming no body");
			return static_cast<int>(request_end + REQUEST_EOF);
		}
		if (content_length == 0)
			return static_cast<int>(request_end + REQUEST_EOF);
		size_t total_length = request_end + REQUEST_EOF + content_length;
		if (_data_in.length() >= total_length) {
			return static_cast<int>(total_length);
		}
		return 0;
	}
	return static_cast<int>(request_end + REQUEST_EOF);
}
