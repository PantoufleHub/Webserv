#include "HttpUtils.hpp"

/// @brief Makes a generic http response (with body) based on a status code
/// @param status_code Status code of response
/// @param content_type Content type of body
/// @param content Body of the response
/// @return The response as a string
string HttpUtils::makeResponse(int status_code, string content_type, string content) {
	// replace OK with correct message
	ostringstream ss;
	ss << "HTTP/1.1 " << status_code << " " << HttpUtils::getHttpStatusMessage(status_code) << "\r\n"
	   << "Content-Type: " << content_type << "\r\n"
	   << "Content-Length: " << content.length() << "\r\n"
	   << "\r\n"
	   << content;
	return ss.str();
}

/// @brief Makes a generic http response based on a status code
/// @param status_code Status code of response
/// @return The response as a string
string HttpUtils::makeResponse(int status_code) {
	// replace OK with correct message
	ostringstream ss;
	ss << "HTTP/1.1 " << status_code << " " << HttpUtils::getHttpStatusMessage(status_code) << "\r\n"
	   << "Content-Length: 0" << "\r\n"
	   << "\r\n";
	return ss.str();
}

string HttpUtils::_getDefaultErrorPage(int status_code) {
	stringstream status_code_ss;
	status_code_ss << status_code;
	return "<html><body><h1>" + status_code_ss.str() + " " + HttpUtils::getHttpStatusMessage(status_code) + "</h1></body></html>";
}

void HttpUtils::getErrorPage(HttpResponse &response, const Location& location, int status_code) {
	// TEMP
	response.setStatusCode(status_code);
	const map<int, string> errors = location.getErrors();
	string root = location.getRoot();
	if (errors.find(status_code) != errors.end()) {
		// Attention code dégeu ahead
		cout << "Custom error page found for status code " << status_code << ": " << errors.at(status_code) << endl;
		string str_error_file = errors.at(status_code);
		str_error_file = (str_error_file[0] != '/') ? "/" + str_error_file : str_error_file;
		string str_full_path = (root + str_error_file);
		const char* error_file = str_full_path.c_str();
		cout << "Using custom error page: " << error_file << endl;
		ifstream page(error_file);
		if (page.is_open()) {
			ostringstream osstrpage;
			osstrpage << page.rdbuf();
			string content = osstrpage.str();
			response.setBody("text/html", content);
			return;
		} else {
			cout << "Error opening error page " << error_file << endl;
		}
	}
	response.setBody("text/html", _getDefaultErrorPage(status_code));
	return;
}

const string HttpUtils::getHttpStatusMessage(int status_code) {
	switch (status_code) {
		case 200:
			return "OK";
		case 201:
			return "Created";
		case 204:
			return "No content";
		case 301:
			return "Moved Permanently";
		case 302:
			return "Found";
		case 400:
			return "Bad Request";
		case 403:
			return "Forbidden";
		case 404:
			return "Not Found";
		case 405:
			return "Method Not Allowed";
		case 408:
			return "Request Timeout";
		case 409:
			return "Conflict";
		case 411:
			return "Length Required";
		case 413:
			return "Payload Too Large";
		case 414:
			return "URI Too Long";
		case 415:
			return "Unsupported Media Type";
		case 429:
			return "Too Many Requests";
		case 503:
			return "Service Unavailable";
		case 505:
			return "Http Version Not Supported";
		case 500:
			return "Internal Server Error";
		default:
			return "Internal Server Error";
	}
}

string HttpUtils::_getAutoIndexPageFooter() {
	return "</body>\n</html>\n";
}

string HttpUtils::_getAutoIndexPageHeader(const string& path) {
	return "<html>\n"
	       "<head><title>Autoindex of " +
	       path +
	       "</title></head>\n"
	       "<body>\n"
	       "	<h1>Autoindex of " +
	       path + "</h1><br/>\n";
}

string HttpUtils::_getAutoIndexEntry(const string& path, const string& entry, bool isDir) {
	(void)path;
	return "<a href=\"" + entry + (isDir ? "/" : "") + "\">" + entry + (isDir ? "/" : "") + "</a><br/>\n";
}

void HttpUtils::getAutoIndexPage(HttpResponse &response, const Location& location, const string& path) {
	// AUTOINDEX ADD LINKS!
	// ret.setBody(TYPE_HTML, "This is a placeholder for an autoindex page.\nPath: " + path + "\n");

	DIR* dir = opendir(path.c_str());
	if (!dir) {
		std::cout << "Could not open directory: " << path << std::endl;
		HttpUtils::getErrorPage(response, location, HTTP_CODE_FORBIDDEN);
		return;
	}

	response.setBody(TYPE_HTML, _getAutoIndexPageHeader(path));
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		std::string entry_name = entry->d_name;

		if (entry_name == "." || entry_name == "..")
			continue;

		std::string full_path = path + "/" + entry_name;

		struct stat info;
		if (stat(full_path.c_str(), &info) != 0) {
			std::cout << "Stat failed for: " << full_path << std::endl;
			continue;
		}

		bool isDir = S_ISDIR(info.st_mode);
		response.addBody(TYPE_HTML, _getAutoIndexEntry(path, entry_name, isDir));
	}
	closedir(dir);
	response.addBody(TYPE_HTML, _getAutoIndexPageFooter());
}

/// @brief Gets a chunk of data from a http body
/// @param body The body of the http message
/// @param pos The position from which to start parsing
/// @return The retrieved chunked from the body in string format
string HttpUtils::unchunkString(const string &body, size_t &pos) {
	if (pos >= body.size()) {
		return "";
	}

	size_t find_crlf = body.find("\r\n", pos);
	if (find_crlf == string::npos) {
		return "";
	}

	string hex_size = body.substr(pos, find_crlf - pos);
	size_t semicolon = hex_size.find(';');
	if (semicolon != string::npos) {
		hex_size = hex_size.substr(0, semicolon);
	}

	size_t first = hex_size.find_first_not_of(" \t");
	size_t last = hex_size.find_last_not_of(" \t");
	if (first != string::npos && last != string::npos) {
		hex_size = hex_size.substr(first, last - first + 1);
	}

	if (!StringUtils::is_hex_string(hex_size)) {
		cout << "Not a hex string: " << hex_size << "\n";
		return "";
	}

	size_t chunk_size = strtol(hex_size.c_str(), NULL, 16);
	if (chunk_size == 0) {
		if (body.size() >= find_crlf + 4 && 
			body.substr(find_crlf + 2, 2) == "\r\n") {
			pos = body.size();
		}
		return "";
	}

	size_t chunk_start = find_crlf + 2;
	size_t chunk_end = chunk_start + chunk_size;

	if (chunk_end + 2 > body.size()) {
		cout << "Incomplete chunk\n";
		return "";
	}

	string chunk = body.substr(chunk_start, chunk_size);
	pos = chunk_end + 2;

	return chunk;
}

/// @brief Cut a string into chunks
/// @param body The full body to chunk
/// @param pos The position to start chunking from
/// @param length The max length of the chunk
/// @param return_chunk The chunk returned
/// @return The size of the chunk returned
size_t HttpUtils::chunkString(const string &body, size_t &pos, size_t length, string &return_chunk) {
	if (length == 0 || pos > body.size())
	return 0;

	return_chunk.clear();

	if (pos == body.size()) {
		return_chunk = "0\r\n\r\n";
		pos++;
		cout << "Reached end of file" << endl;
		return return_chunk.size();
	}

	stringstream strstr_chunk;
	string body_part = body.substr(pos, length);
	strstr_chunk << hex << body_part.size() << "\r\n" << body_part << "\r\n";
	return_chunk = strstr_chunk.str();
	pos += length;

	if (pos > body.size())
		pos = body.size();

	cout << "Chunked " << return_chunk.size() << " bytes" << endl;
	return return_chunk.size();
}

string HttpUtils::getStringInChunk(const string& chunk) {
	size_t size = chunk.size();
	stringstream retstrstr; //nice name huh

	retstrstr << hex << size << "\r\n" << chunk << "\r\n";
	return retstrstr.str();;
}

/// @brief Cut the contents of a file into chunks
/// @param fd The file to chunk
/// @param chunk_size The max size of the chunk
/// @param return_chunk The chunk returned
/// @return The number of bytes read from the fd
size_t HttpUtils::chunkFile(int fd, size_t chunk_size, string &return_chunk) {
	return_chunk.clear();
	
	if (chunk_size == 0)
	return 0;

	char buffer[chunk_size + 1];
	bzero(buffer, chunk_size + 1);
	ssize_t bytes_read = read(fd, buffer, chunk_size);

	if (bytes_read < 0) {
		cout << "Read error" << endl;
		return 0;
	} else if (bytes_read == 0) {
		return_chunk = "0\r\n\r\n";
		cout << "Reached end of file" << endl;
		return 0;
	}

	stringstream strstr_chunk;
	string body_part(buffer, bytes_read);
	strstr_chunk << hex << body_part.size() << "\r\n" << body_part << "\r\n";
	return_chunk = strstr_chunk.str();

	cout << "Chunked " << bytes_read << " bytes" << endl;
	return bytes_read;
}

/// @brief Writes data to a file
/// @param fd The file to write to
/// @param body The content to wite
/// @param pos The position in body to start writing from
/// @param buffer_size The max amount of bytes to write
/// @return The number of bytes written
ssize_t HttpUtils::write_data(const int fd, const string body, size_t &pos, size_t buffer_size) {
	ssize_t bytes_read;

	if (pos >= body.size())
		return 0;
	if (pos + buffer_size > body.size())
		buffer_size = body.size() - pos;
	bytes_read = write(fd, &body[pos], buffer_size);
	pos += buffer_size;

	cout << "Wrote " << bytes_read << " bytes to file" << endl;
	return bytes_read;
}

