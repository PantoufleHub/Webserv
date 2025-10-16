#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <cstdlib>

#include "HttpRequest.hpp"
#include "HttpUtils.hpp"
#include "Constants.hpp"

class HttpRequestParser {
   private:
	static string _getLine(istringstream& iss);
	static bool _parseCLRF(string& str);
	static bool _parseFirstLine(HttpRequest& request, istringstream& iss);
	static bool _parseHeaders(HttpRequest& request, istringstream& iss);
	static bool _parseBody(HttpRequest& ret, istringstream& iss);
	static bool _validateRequest(HttpRequest& request);

   public:
	static HttpRequest* strToHttpRequest(string str);
	static int checkDataIn(const string& _data_in);
};
