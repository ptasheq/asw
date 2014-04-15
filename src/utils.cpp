#include <sstream>
#include <iostream>
#include <mpi.h>
#include <cstdlib>

namespace Utils {

int checkArguments(int argc, char ** argv) {
	if (argc != 2) {
		std::cerr << "Usage: exec <workers number>" << std::endl;
		exit(EXIT_FAILURE);
	}

	std::istringstream iss(argv[1]);
	int val;

	if (!(iss >> val)) {
		std::cerr << "Usage: exec <workers number>" << std::endl;
		exit(EXIT_FAILURE);
	}
	return val;
}

}