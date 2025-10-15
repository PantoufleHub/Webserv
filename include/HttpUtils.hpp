#pragma once

#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>

#include "Location.hpp"
#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "Constants.hpp"
#include "Logger.hpp"
#include "StringUtils.hpp"

class HttpResponse;
class Location;

class HttpUtils {
   private:
	HttpUtils();
	static string _getAutoIndexEntry(const string& path, const string& entry, bool isDir);
	static string _getAutoIndexPageHeader(const string& path);
	static string _getAutoIndexPageFooter();
	static string _getDefaultErrorPage(int status_code);

   public:
	static string makeResponse(int status_code);
	static string makeResponse(int status_code, string content_type, string content);
	static const string getHttpStatusMessage(int status_code);
	static void getErrorPage(HttpResponse &response, const Location& location, int status_code);
	static void getAutoIndexPage(HttpResponse &response, const Location& location, const string& path);
	static size_t chunkFile(int fd, size_t chunk_size, string &return_chunk);
	static string unchunkString(const string &body, size_t &pos);
	static size_t chunkString(const string &body, size_t &pos, size_t length, string &return_chunk);
	static string getStringInChunk(const string& chunk);
	static ssize_t write_data(const int fd, const string body, size_t &pos, size_t buffer_size);
};
