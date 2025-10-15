#include "EntryPoint.hpp"

bool EntryPoint::operator==(const EntryPoint& other) const {
	return (ip == "0.0.0.0" || other.ip == "0.0.0.0" || ip == other.ip) && port == other.port;
}

EntryPoint& EntryPoint::operator=(const EntryPoint& other) {
	if (this != &other) {
		ip = other.ip;
		port = other.port;
	}
	return *this;
}

/// @brief Verifies if an Entry point is contained in a vector of Entry points
/// @param ep Entry point to look for
/// @param eps Vector of Entry points to look in
/// @return True if found
bool EpInEps(const EntryPoint& ep, const vector<EntryPoint>& eps) {
	for (size_t i = 0; i < eps.size(); i++) {
		if (ep == eps[i])
			return true;
	}
	return false;
}

ostream& operator<<(ostream& os, const EntryPoint& ep) {
	os << ep.ip << ":" << ep.port;
	return os;
}
