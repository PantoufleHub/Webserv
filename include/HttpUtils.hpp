#pragma once

#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <dirent.h>

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
};
