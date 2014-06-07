#include "process.h"
#include "protocol.h"
#include "utils.h"
#include "consts.h"
#include <iostream>
#include <mpi.h>
#include <unistd.h>
#include <vector>
#include <cstdlib>
#include <ctime>

void Process::dispatchMessage(MPI_Status * status) {
}

int Process::run(int rank, int size, int val) {

	int flag;
	this->workerCount = val;
	this->size = size;
	this->rank = rank;
	int it = 0;
	MPI_Status status;

	srand(time(NULL));

	while (it < Utils::settings.iterations) {
		MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
		while (flag) {
			this->dispatchMessage(&status);
			MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
		}
		this->performAction();
		sleep(1);
	}

	MPI_Finalize();
	return 0;
}

void Process::performAction() {
}

void Process::showIdentity() {
}

SocialWorker::SocialWorker() {
	this->myState = SEARCHING_FOR_PAIR;
}

SocialWorker::~SocialWorker() {
}

void SocialWorker::dispatchMessage(MPI_Status * status) {
	int msg;
	MPI_Recv(&msg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
	std::cout << "message received" << std::endl;
}

SocialWorker & SocialWorker::getInstance() {
	static SocialWorker instance;
	return instance;
}

void SocialWorker::performAction() {
	MPI_Status status;
	// making the pairs
	switch(this->myState) {
		case SEARCHING_FOR_PAIR: {
			std::vector<int> alcoholics;
			for (int i = this->workerCount; i < this->size; ++i) alcoholics.push_back(i); 
			std::cout << "Worker " <<this->rank << " searching for pair, " <<this->workerCount << " " << this->size << std::endl;
			int processIter = rand() % alcoholics.size();

			while (alcoholics.size() > 0) {
				std::cout << std::endl;
				msg = this->clk;
				MPI_Send(&msg, 1, MPI_INT, alcoholics[processIter], WANNA_DRINK, MPI_COMM_WORLD);
				std::cout << "Worker " << this->rank<< " : waiting for message from " << alcoholics[processIter] << std::endl;
				this->waitForMessageFrom(alcoholics[processIter]);
				this->dispatchMessage(&status);
				if (status.MPI_TAG == SURE) {
					std::cout << "Worker: waiting in queue with " << alcoholics[processIter] << std::endl;
					this->partnerRank = status.MPI_SOURCE;
					this->myState = SEARCHING_FOR_PUB;
					break;
				}
				alcoholics.erase(alcoholics.begin() + processIter);
				processIter = rand() % alcoholics.size();
			}
		break;
		}
		case SEARCHING_FOR_PUB:
			//TODO: searching for pub
		break;

	}
}

void SocialWorker::showIdentity() {
	std::cout << "I'm a social worker" << std::endl;
}

void SocialWorker::waitForMessageFrom(int processId) {
	MPI_Status status;
	int flag;
	MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
	while (status.MPI_SOURCE != processId) {
		// if somebody else sent us a message
		if (flag) {
			this->dispatchMessage(&status);
		}
		MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
	}
}

Alcoholic::Alcoholic() {
	this->myState = WAITING_FOR_PAIR;
}

Alcoholic::~Alcoholic() {
	
}

void Alcoholic::dispatchMessage(MPI_Status * status) {
	switch (status->MPI_TAG) {
		case WANNA_DRINK:
			if (myState == WAITING_FOR_PAIR) {
				myState = IN_PAIR;
				MPI_Send(&this->clk, 1, MPI_INT, status->MPI_SOURCE, SURE, MPI_COMM_WORLD); 
			}
			else {
				MPI_Send(&this->clk, 1, MPI_INT, status->MPI_SOURCE, NOPE, MPI_COMM_WORLD); 
			}
		break;
	}
	MPI_Recv(&msg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
	std::cout << "message received" << std::endl;
}

Alcoholic & Alcoholic::getInstance() {
	static Alcoholic instance;
	return instance;
}

void Alcoholic::performAction() {
	MPI_Status status;
}

void Alcoholic::showIdentity() {
	std::cout << "I'm an alcoholic" << std::endl;
}