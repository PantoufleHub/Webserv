#pragma once

#include <string>
#include <vector>
#include <iostream>

using namespace std;

// Represents ip and port listen blocks
struct EntryPoint {
	string ip;
	int port;
	bool operator==(const EntryPoint& other) const;
	EntryPoint& operator=(const EntryPoint& other);
};

bool EpInEps(const EntryPoint& ep, const vector<EntryPoint>& eps);
ostream& operator<<(ostream& os, const EntryPoint& ep);
