#pragma once

#include <vector>

#include "ABlockDirective.hpp"
#include "Location.hpp"

struct EntryPoint {
	std::string ip;
	int port;
	int backlog;

	bool operator==(const EntryPoint& other) const;
};

bool EntryPointInList(const EntryPoint& ep, const std::vector<EntryPoint>& list);

class VirtualServer : public ABlockDirective {
   private:
	std::vector<Location> _locations;
	std::vector<EntryPoint> _entry_points;

   public:
	VirtualServer();
	~VirtualServer();

	const std::vector<EntryPoint>& getEntryPoints() const;
	const std::vector<Location>& getLocations() const;
	std::vector<Location>& mutableLocations();

	void setLocations(Location& location);
	void addEntryPoint(const EntryPoint& ep);
};
