#ifndef PROCESS_H
#define PROCESS_H

#include <mpi.h>
#include <stack>

class Process {
protected:
	int rank, size, clk, workerCount;
	int partnerRank, myState;
	int msg;
	std::stack<int> waitingForAccept;
public:
	virtual void dispatchMessage(MPI_Status *, int *);
	virtual void performAction();
	virtual int run(int, int, int);
	virtual void showIdentity();
};

class Alcoholic: public Process {
private:
	Alcoholic();
	~Alcoholic();
	int remainRestTime;
public:
	void dispatchMessage(MPI_Status *, int *);
	static Alcoholic & getInstance();
	void performAction();
	void showIdentity();
};

class SocialWorker: public Process {
private:
	SocialWorker();
	~SocialWorker();
	int myPub, remainDrinkTime;
	int * pubCapacities;
public:
	void dispatchMessage(MPI_Status *, int *);
	static SocialWorker & getInstance();
	void performAction();
	void showIdentity();
	void waitForMessageFrom(int);
};

#endif