#include "Constants.hpp"

#include "DirectiveKeys.hpp"

const std::map<std::string, int>& getDirectivesMap() {
	static std::map<std::string, int> directives;
	if (directives.empty()) {
		directives["none"] = 8192;  // kept for completeness with legacy parser
		directives["autoindex"] = autoindex;
		directives["return"] = redirection;
		directives["upload_store"] = upload_store;
		directives["server_name"] = server_name;
		directives["listen"] = listen_config;
		directives["location"] = location_config;
		directives["root"] = root_config;
		directives["cgi_pass"] = cgi_pass;
		directives["index"] = index_config;
		directives["error"] = error_config;
		directives["allow_method"] = allow_method_config;
		directives["client_max_body_size"] = client_max_body_size;
	}
	return directives;
}
