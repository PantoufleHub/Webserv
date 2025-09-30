#include "VirtualServer.hpp"

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
	_locations.push_back(location);
}

void VirtualServer::addEntryPoint(const EntryPoint& ep) {
	_entry_points.push_back(ep);
}

bool EntryPoint::operator==(const EntryPoint& other) const {
	return (ip == "0.0.0.0" || other.ip == "0.0.0.0" || ip == other.ip) && port == other.port;
}

bool EntryPointInList(const EntryPoint& ep, const std::vector<EntryPoint>& list) {
	for (std::vector<EntryPoint>::const_iterator it = list.begin(); it != list.end(); ++it) {
		if (ep == *it) {
			return true;
		}
	}
	return false;
}
