#pragma once

#include <map>
#include <string>
#include <vector>

class Location;
class VirtualServer;
class ConfigParser;

class ABlockDirective {
   protected:
	std::map<int, std::string> _errors;
	std::map<int, std::string> _redirect;
	std::vector<std::string> _allowed_methods;
	std::vector<std::string> _indexes;
	std::vector<std::string> _names;
	std::vector<int> _ports;
	std::string _upload_store;
	std::string _root;
	size_t	_client_max_body_size;
	bool	_autoindex;

   public:
	ABlockDirective();
	virtual ~ABlockDirective();

	const std::map<int, std::string>& getErrors() const;
	const std::map<int, std::string>& getRedirect() const;
	const std::vector<std::string>& getAllowedMethods() const;
	const std::vector<std::string>& getIndexes() const;
	const std::vector<std::string>& getNames() const;
	const std::vector<int>& getPorts() const;
	const std::string& getRoot() const;
	const std::string& getUploadStore() const;
	const size_t& getClientMaxBodySize() const;
	const bool& getAutoIndex() const;

	std::vector<std::string>& mutableIndexes();
	std::map<int, std::string>& mutableErrors();

	void setBlockDirective(std::vector<std::string>::iterator& it);
	void addRootPrefixToErrors();
	void addRootPrefixToVectorOfString(std::vector<std::string>& path);
	void addRootSuffix();
	void setAllowedMethods(std::vector<std::string>::iterator& it);
	void setIndexes(std::vector<std::string>::iterator& it);
	void setNames(std::vector<std::string>::iterator& it);
	void setPorts(std::vector<std::string>::iterator& it);
	void setErrors(std::vector<std::string>::iterator& it);
	void setClientMaxBodySize(std::vector<std::string>::iterator& it);
	void setClientMaxBodySize();
	void setUploadStore(std::vector<std::string>::iterator& it);
	void setAutoIndex(std::vector<std::string>::iterator& it);
	void setRedirect(std::vector<std::string>::iterator& it);
	void setRoot(std::vector<std::string>::iterator& it);
};
