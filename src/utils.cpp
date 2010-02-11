#include "utils.hpp"
#include <sstream>

/** from stroustrup c++ technique FAQ */
std::string itos(int i) {
	std::stringstream s;
	s << i;
	return s.str();
}
