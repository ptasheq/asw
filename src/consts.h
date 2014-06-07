#ifndef CONSTS_H
#define CONSTS_H

#define FAIL -1
#define CONFIGFILE_PATH "config/config.cfg"
#define DEFAULT_PUB_NUMBER 2
#define DEFAULT_SOBER_STATION_CAPACITY 5
#define DEFAULT_ITERATIONS 1000 
extern const int DEFAULT_PUB_CAPACITY[]; 

#define SLEEP_TIME 100
#define MIN_PUB_TIME 3000
#define TOLERANCE_PUB_TIME 3000


enum WorkerStates {SEARCHING_FOR_PAIR=0, SEARCHING_FOR_PUB, IN_PUB};
enum AlcoholicStates {WAITING_FOR_PAIR=0, IN_PAIR};

#endif