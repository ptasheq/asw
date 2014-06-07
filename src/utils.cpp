#include <sstream>
#include <iostream>
#include <mpi.h>
#include <cstdlib>
#include <fstream>
#include "consts.h"
#include "process.h"
#include "utils.h"

const int DEFAULT_PUB_CAPACITY[] = {3, 4};

namespace Utils {

Settings settings;

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

int loadSettings() {
	std::ifstream configFile (CONFIGFILE_PATH, std::ifstream::in);	

	// config file doesn't exist we use deafult settings
	if (configFile.fail()) {
		settings.pubCount = DEFAULT_PUB_NUMBER;
		settings.pubCapacity.resize(settings.pubCount); 
		for (int i = 0; i < settings.pubCount; ++i) {
			settings.pubCapacity[i] = DEFAULT_PUB_CAPACITY[i];
		}
		settings.soberStationCapacity = DEFAULT_SOBER_STATION_CAPACITY;
		settings.iterations = DEFAULT_ITERATIONS;
	}
	else {
		configFile >> settings.pubCount;
		settings.pubCapacity.resize(settings.pubCount);
		for (int i = 0; i < settings.pubCount; ++i) {
			configFile >> settings.pubCapacity[i];
		}
		configFile >> settings.soberStationCapacity;
		configFile >> settings.iterations;
		configFile.close();
	}
	return 0;
}

}
