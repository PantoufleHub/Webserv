#pragma once

#include <cctype>
#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

class StringUtils {
   public:
	static std::string sizetToString(std::size_t value);
	static bool strInStrs(const std::string& str, const std::vector<std::string>& strs);
	static bool insCompare(const std::string& str1, const std::string& str2);
	static std::size_t nbCharsInStr(char c, const std::string& str);
};
