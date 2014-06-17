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

#define MIN_REST_TIME 2000
#define TOLERANCE_REST_TIME 4000

#define NOT_IN_PUB -1
#define NO_PARTNER -1

enum WorkerStates {
	SEARCHING_FOR_PAIR=(1 << 0), 
	SEARCHING_FOR_PUB=(1 << 1), 
	IN_PUB=(1 << 2), 
	WORKER_FINISHING=(1 << 3)
};
enum AlcoholicStates {
	WAITING_FOR_PAIR=(1 << 4), 
	IN_PAIR=(1 << 5), 
	WAITING_FOR_SOBER_STATION=(1 << 6), 
	IN_SOBER_STATION=(1 << 7), 
	ALCOHOLIC_FINISHING=(1 << 8)
};

enum UniversalStates {FINISH=(1 << 16)};

#endif
