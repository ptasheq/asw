#ifndef UTILS_H
#define UTILS_H

#include <vector>

namespace Utils {

	struct Settings {
	    int pubCount;
	    std::vector<int> pubCapacity;
	    int soberStationCapacity;
	    int iterations;
	};

	extern Settings settings;

	int checkArguments(int, char **);
	int loadSettings();
	void msleep(int);

}

#endif
