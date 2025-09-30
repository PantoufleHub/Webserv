#pragma once

#include <map>
#include <string>
#include <vector>

#include "ABlockDirective.hpp"

class VirtualServer;

class Location : public ABlockDirective {
   private:
	std::map<std::string, std::vector<std::string> > _cgi_directives;

   public:
	Location();
	~Location();

	const std::map<std::string, std::vector<std::string> >& getCgi() const;
	void inheritFromParent(const VirtualServer& parent);
	void setCgi(std::vector<std::string>::iterator& it);
	bool checkMethod(const std::string& method) const;
};
