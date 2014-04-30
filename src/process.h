#ifndef PROCESS_H
#define PROCESS_H

#include <mpi.h>

struct Settings {
    int pubCount;
    int * pubCapacity;
    int soberStationCapacity;
};

class Process {
protected:
	int rank, size;
	int msg;
	Settings * settings;
public:
	virtual void dispatchMessage(MPI_Status * status);
	virtual void performAction();
	virtual int run(int, int);
	virtual void showIdentity();
};

class Alcoholic: public Process {
private:
	Alcoholic();
	~Alcoholic();
public:
	void dispatchMessage(MPI_Status * status);
	static Alcoholic & getInstance();
	void performAction();
	void showIdentity();
};

class SocialWorker: public Process {
private:
	SocialWorker();
	~SocialWorker();
public:
	void dispatchMessage(MPI_Status * status);
	static SocialWorker & getInstance();
	void performAction();
	void showIdentity();
};

#endif