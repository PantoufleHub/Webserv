#include "StringUtils.hpp"

std::string StringUtils::sizetToString(std::size_t value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

bool StringUtils::strInStrs(const std::string& str, const std::vector<std::string>& strs) {
	for (std::vector<std::string>::const_iterator it = strs.begin(); it != strs.end(); ++it) {
		if (*it == str) {
			return true;
		}
	}
	return false;
}

bool StringUtils::insCompare(const std::string& str1, const std::string& str2) {
	if (str1.length() != str2.length()) {
		return false;
	}
	for (std::size_t i = 0; i < str1.length(); ++i) {
		if (std::tolower(str1[i]) != std::tolower(str2[i])) {
			return false;
		}
	}
	return true;
}

std::size_t StringUtils::nbCharsInStr(char c, const std::string& str) {
	std::size_t count = 0;
	for (std::size_t i = 0; i < str.length(); ++i) {
		if (str[i] == c) {
			++count;
		}
	}
	return count;
}
