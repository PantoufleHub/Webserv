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
			cerr << "Error opening error page " << error_file << endl;
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
		std::cerr << "Could not open directory: " << path << std::endl;
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
			std::cerr << "Stat failed for: " << full_path << std::endl;
			continue;
		}

		bool isDir = S_ISDIR(info.st_mode);
		response.addBody(TYPE_HTML, _getAutoIndexEntry(path, entry_name, isDir));
	}
	closedir(dir);
	response.addBody(TYPE_HTML, _getAutoIndexPageFooter());
}
