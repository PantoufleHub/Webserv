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
	static bool is_hex_string(const string &s);
	static string trimSlashes(const string& input, bool trim_start, bool trim_end);
	static string pathConcatenate(const string& path1, const string& path2, bool starting_slash, bool ending_slash);
	static string pathConcatenate(const string& path1, const string& path2, bool trim_start, bool trim_end);
};
