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
	const string raw_request = iss.str();
	std::streampos body_pos = iss.tellg();
	if (body_pos == std::streampos(-1)) {
		body_pos = static_cast<std::streampos>(raw_request.size());
	}
	string remaining = raw_request.substr(static_cast<size_t>(body_pos));

	if (request.isHeaderSet("Transfer-Encoding") && 
		StringUtils::insCompare(request.getHeaderValue("Transfer-Encoding"), "chunked")) {
		string unchunked_body;
		size_t pos = 0;
		string chunk;

		while (!(chunk = HttpUtils::unchunkString(remaining, pos)).empty()) {
			unchunked_body += chunk;
		}
		request.setBody(unchunked_body);
		ostringstream content_length;
		content_length << unchunked_body.size();
		request.addHeader("Content-Length", content_length.str());
		
	} else {
		request.setBody(remaining);
	}
	return true;
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

HttpRequest* HttpRequestParser::strToHttpRequest(string str) {
	HttpRequest* ret = new HttpRequest();
	istringstream iss(str);

	(*ret).flagAsInvalid();

	if (!_parseFirstLine(*ret, iss))
		return ret;

	if (!_parseHeaders(*ret, iss))
		return ret;

	if (!_parseBody(*ret, iss))
		return ret;

	if (!_validateRequest(*ret))
		return ret;

	(*ret).flagAsValid();
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

static bool isChunkedEncoding(const string& dataIn) {
	size_t pos = 0;
	while ((pos = dataIn.find("Transfer-Encoding:", pos)) != string::npos ||
			(pos = dataIn.find("transfer-encoding:", pos)) != string::npos) {
		size_t line_end = dataIn.find("\r\n", pos);
		if (line_end == string::npos) {
			return false;
		}
		size_t value_start = dataIn.find(":", pos) + 1;
		string value = dataIn.substr(value_start, line_end - value_start);
		size_t first = value.find_first_not_of(" \t");
		size_t last = value.find_last_not_of(" \t");
		if (first != string::npos && last != string::npos) {
			value = value.substr(first, last - first + 1);
			if (value == "chunked") {
				return true;
			}
		}
		pos = line_end;
	}
	return false;
}

static int checkChunkedRequest(const string& dataIn, size_t headers_end) {
	size_t pos = headers_end + 4;

	while (pos < dataIn.size()) {
		size_t chunk_size_end = dataIn.find("\r\n", pos);
		if (chunk_size_end == string::npos) {
			return 0;
		}
		string chunk_size_str = dataIn.substr(pos, chunk_size_end - pos);
		size_t semicolon = chunk_size_str.find(';');
		if (semicolon != string::npos) {
			chunk_size_str = chunk_size_str.substr(0, semicolon);
		}
		if (!StringUtils::is_hex_string(chunk_size_str)) {
			Logger::logError("Invalid chunk size: " + chunk_size_str);
			return -1;
		}
		char* end_ptr;
		long chunk_size = strtol(chunk_size_str.c_str(), &end_ptr, 16);
		if (*end_ptr != '\0' || chunk_size < 0) {
			Logger::logError("Failed to parse chunk size");
			return -1;
		}
		if (chunk_size == 0) {
			size_t final_crlf = chunk_size_end + 2;
			if (final_crlf + 2 > dataIn.size()) {
				return 0;
			}
			return static_cast<int>(final_crlf + 2);
		}
		size_t chunk_data_start = chunk_size_end + 2;
		size_t chunk_data_end = chunk_data_start + chunk_size;
		if (chunk_data_end + 2 > dataIn.size()) {
			return 0;
		}
		if (dataIn[chunk_data_end] != '\r' || dataIn[chunk_data_end + 1] != '\n') {
			Logger::logError("Missing CRLF after chunk data");
			return -1;
		}
		pos = chunk_data_end + 2;
	}
	return 0;
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
		if (isChunkedEncoding(_data_in)) {
			return checkChunkedRequest(_data_in, request_end);
		}
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
