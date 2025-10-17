#include "VirtualServer.hpp"
#include "Logger.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

VirtualServer::VirtualServer() : ABlockDirective() {}
VirtualServer::~VirtualServer() {}

const std::vector<EntryPoint>& VirtualServer::getEntryPoints() const {
	return _entry_points;
}
const std::vector<Location>& VirtualServer::getLocations() const {
	return _locations;
}
std::vector<Location>& VirtualServer::mutableLocations() {
	return _locations;
}

void VirtualServer::setLocations(Location& location) {
	const string& new_location_name = location.getNames()[0];

	for (size_t i = 0; i < _locations.size(); i++) {
		if (_locations[i].getNames()[0] == new_location_name) {
			ostringstream warning;
			warning << "Duplicate location block: " << new_location_name
					<< " (previous definition will be overwritten)";
			Logger::logError(warning.str());
			_locations[i] = location;
			return ;
		}
	}
	_locations.push_back(location);
}

void VirtualServer::addEntryPoint(const EntryPoint& ep) {
	_entry_points.push_back(ep);
}
