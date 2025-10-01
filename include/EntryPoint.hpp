#pragma once

#include <string>
#include <vector>

using namespace std;

// Represents ip and port listen blocks
struct EntryPoint {
	string ip;
	int port;
	int backlog;  // number of possible simultaneous pending connections
	bool operator==(const EntryPoint& other) const;
};
bool EpInEps(const EntryPoint& ep, const vector<EntryPoint>& eps);
