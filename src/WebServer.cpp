#include <WebServer.hpp>

#include <ConfigParser.hpp>
#include <VirtualServer.hpp>

WebServer::WebServer(const string& config_file) {
	ConfigParser parsedFile(config_file);
	parsedFile.tokeniseConfigFile();

	_virtual_servers = parsedFile.parseTokens(parsedFile.getTokens());
}

WebServer::~WebServer() {
	
}

void WebServer::display() const {
	for (size_t i = 0; i < _virtual_servers.size(); i++) {
		cout << "Server: " << _virtual_servers[i].getNames()[0] << " ; Root: " << _virtual_servers[i].getRoot() << endl;
		map<int, string> errors = _virtual_servers[i].getErrors();
		vector<string> indexes = _virtual_servers[i].getIndexes();
		vector<string> allowed_methods = _virtual_servers[i].getAllowedMethods();
		vector<Location> locations = _virtual_servers[i].getLocations();
		vector<EntryPoint> eP = _virtual_servers[i].getEntryPoints();
		cout << "\nErrors: " << endl;
		for (map<int, string>::iterator it = errors.begin(); it != errors.end(); it++) {
			cout << "Error " << it->first << "->" << it->second << endl;
		}
		cout << "\nIndexes: " << endl;
		for (vector<string>::iterator it = indexes.begin(); it != indexes.end(); it++) {
			cout << *it << endl;
		}
		cout << "\nAllowed_methods: " << endl;
		for (vector<string>::iterator it = allowed_methods.begin(); it != allowed_methods.end(); it++) {
			cout << *it << endl;
		}
		for (size_t i = 0; i < locations.size(); i++) {
			cout << "\nLocations" << "[" << i << "]: " << flush;
			map<string, vector<string> > cgi = locations[i].getCgi();
			map<int, string> redirect = locations[i].getRedirect();
			if (cgi.empty()) {
				cout << locations[i].getNames()[0] << "\nRoot: " << locations[i].getRoot()
				     << "\nRedirect: " << locations[i].getNames()[0] << endl;
				for (map<int, string>::iterator itRedir = redirect.begin(); itRedir != redirect.end(); itRedir++) {
					cout << "Key: " << itRedir->first << " Values: " << itRedir->second << flush;
				}
				cout << "\nAutoIndex: " << locations[i].getAutoIndex()
				     << "\nUpload Store: " << locations[i].getUploadStore() << "\nAllowed Methods: " << endl;
				for (size_t it = 0; it < locations[i].getAllowedMethods().size(); it++)
					cout << locations[i].getAllowedMethods()[it] << endl;
			} else {
				cout << "Cgi: " << locations[i].getNames()[0] << endl;
				for (map<string, vector<string> >::iterator itCgi = cgi.begin(); itCgi != cgi.end(); itCgi++) {
					cout << "Key: " << itCgi->first << " Values: " << flush;
					for (size_t i = 0; i < itCgi->second.size(); i++)
						cout << itCgi->second[i] << "; " << flush;
					cout << endl;
				}
			}
		}
		cout << "\nEntryPoints:\n" << flush;
		for (size_t i = 0; i < eP.size(); i++) {
			cout << "EntryPoint " << i << ":\nPort: " << eP[i].port << " IP: " << eP[i].ip << endl;
		}
		cout << endl;
	}
}
