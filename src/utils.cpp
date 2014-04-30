#include <sstream>
#include <iostream>
#include <mpi.h>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include "consts.h"
#include "process.h"

const int DEFAULT_PUB_CAPACITY[] = {10};

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

int loadSettings(Settings * settings) {

	if (!settings) {
		settings = new Settings;
	}
    std::ifstream configFile (CONFIGFILE_PATH, std::ios::in);

    // problem with access to config file, we use deafult settings
    if (configFile.fail()) {
    	settings->pubCount = DEFAULT_PUB_NUMBER;
    	settings->pubCapacity = new int [DEFAULT_PUB_NUMBER];
    	std::copy(DEFAULT_PUB_CAPACITY, DEFAULT_PUB_CAPACITY+1, settings->pubCapacity);
    	settings->soberStationCapacity = DEFAULT_SOBERINGUP_STATION_CAPACITY;
    	std::cout << settings->pubCapacity[0] << std::endl;
    }
    // config file seems to be ok
    else {
    	// TODO
    }
}

}