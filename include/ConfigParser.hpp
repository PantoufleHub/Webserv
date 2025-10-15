#pragma once

#include <exception>
#include <map>
#include <string>
#include <vector>

#include "VirtualServer.hpp"

class ConfigParser {
   private:
	std::string _filePath;
	std::vector<std::string> _tokens;

   public:
	explicit ConfigParser(const std::string& filePath);

	void tokeniseConfigFile();
	std::vector<VirtualServer> parseTokens(std::vector<std::string> tokens);
	std::vector<std::string> getTokens() const;

	class ParsingException : public std::exception {
	   public:
		explicit ParsingException(const char* msg);
		virtual ~ParsingException() throw();
		const char* what() const throw();

	   private:
		std::string _message;
	};
};

void fillEntryPoint(std::vector<std::string>::iterator& it, EntryPoint& entryPoint, VirtualServer& block);

template <typename T, typename T2>
void SetDirectives(int key, T& value, T2& block);

#include "ConfigParser.tpp"
