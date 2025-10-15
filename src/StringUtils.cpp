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

/// @brief Trim slashes from a string
/// @param input The string to trim
/// @param trim_start Whether to trim the starting slashes
/// @param trim_end Whether to trim the ending slashes
/// @return The trimmed string
string StringUtils::trimSlashes(const std::string& input, bool trim_start = true, bool trim_end = true) {
	size_t start = 0;
	size_t end = input.length();

	if (trim_start) {
		while (start < end && input[start] == '/') {
			start++;
		}
	}

	if (trim_end) {
		while (end > start && input[end - 1] == '/') {
			end--;
		}
	}

	return input.substr(start, end - start);
}

/// @brief Concatenate two paths with choice of starting/ending slashes
/// @param path1 The first path to concatenate
/// @param path2 The second path to concatenate
/// @param starting_slash Whether the concatenated path starts with a slash
/// @param ending_slash Whether the concatenated path ends with a slash
/// @return The concatenated path
string StringUtils::pathConcatenate(const string& path1, const string& path2, bool starting_slash = false, bool ending_slash = false) {
	string ret;

	string clean_path1 = trimSlashes(path1);
	string clean_path2 = trimSlashes(path2);

	ret = (starting_slash ? "/" : "")
		+ clean_path1
		+ "/"
		+ clean_path2
		+ (ending_slash ? "/" : "");

	return ret;
}

/// @brief Concatenate two paths with choice of trimming starting/ending slashes
/// @param path1 The first path to concatenate
/// @param path2 The second path to concatenate
/// @param trim_start Whether to trim start slashes
/// @param trim_end Whether to trim end slashes
/// @return The concatenated path
string StringUtils::pathConcatenate(const string& path1, const string& path2, bool trim_start = false, bool trim_end = false) {
	string ret;
	string clean_path1 = trimSlashes(path1, trim_start, true);
	string clean_path2 = trimSlashes(path2, true, trim_end);

	ret = clean_path1 + "/" + clean_path2;

	return ret;
}
