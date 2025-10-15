#include "StringUtils.hpp"

string StringUtils::sizetToString(size_t value) {
	ostringstream oss;
	oss << value;
	return oss.str();
}

bool StringUtils::strInStrs(const string& str, const vector<string>& strs) {
	for (vector<string>::const_iterator it = strs.begin(); it != strs.end(); ++it) {
		if (*it == str) {
			return true;
		}
	}
	return false;
}

bool StringUtils::insCompare(const string& str1, const string& str2) {
	if (str1.length() != str2.length()) {
		return false;
	}
	for (size_t i = 0; i < str1.length(); ++i) {
		if (tolower(str1[i]) != tolower(str2[i])) {
			return false;
		}
	}
	return true;
}

size_t StringUtils::nbCharsInStr(char c, const string& str) {
	size_t count = 0;
	for (size_t i = 0; i < str.length(); ++i) {
		if (str[i] == c) {
			++count;
		}
	}
	return count;
}

string StringUtils::escapeNewlines(const string& input) {
    string result;
    for (size_t i = 0; i < input.size(); i++) {
		if (input[i] == '\r' && input[i + 1] == '\n') {
			result += "\\r\\n\n";
			i++;
		} else if (input[i] == '\n') {
            result += "\\n";
        } else if (input[i] == '\r') {
            result += "\\r";
        } else {
            result += input[i];
        }
    }
    return result;
}

bool StringUtils::is_hex_char(char c) {
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool StringUtils::is_hex_string(const string &s) {
	for (size_t i = 0; i < s.size(); i++) {
		if (!is_hex_char(s[i]))
			return false;
	}
	return true;
}

// string StringUtils::pathConcatenate(const string& path1, const string& path2) {
// 	string ret;


// }
