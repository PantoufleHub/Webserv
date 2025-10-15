#pragma once

#include <cctype>
#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class StringUtils {
   public:
	static string sizetToString(size_t value);
	static bool strInStrs(const string& str, const vector<string>& strs);
	static bool insCompare(const string& str1, const string& str2);
	static size_t nbCharsInStr(char c, const string& str);
	static string escapeNewlines(const string& input);
	static bool is_hex_char(char c);
	static bool is_hex_string(const string &s) ;
};
