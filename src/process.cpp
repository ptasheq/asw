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

void Process::dispatchMessage(MPI_Status * status, int * msg) {
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
			this->dispatchMessage(&status, NULL);
			MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
		}
		this->performAction();
		Utils::msleep(SLEEP_TIME);
	}

	MPI_Finalize();
	return 0;
}

void Process::performAction() {
}

void Process::showIdentity() {
}

SocialWorker::SocialWorker() {
	this->myPub = NOT_IN_PUB;
	this->partnerRank = NO_PARTNER;
	this->myState = SEARCHING_FOR_PAIR;
	this->pubCapacities = new int [Utils::settings.pubCount];
	for (int i = 0; i < Utils::settings.pubCount; ++i) {
		this->pubCapacities[i] = 0;
	}
}

SocialWorker::~SocialWorker() {
	delete [] this->pubCapacities;
}

void SocialWorker::dispatchMessage(MPI_Status * status, int * msg) {
	switch(status->MPI_TAG) {
		case ACCEPT: {
			if (!msg) {
				int tmpMsg[2];
				MPI_Recv(tmpMsg, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
			}
			else {
				MPI_Recv(msg, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
			}
		}
		break;
		case CAN_ENTER: {
			std::cout << "Worker " << this->rank << " got request from " << status->MPI_SOURCE << std::endl;
			int tmpMsg[2];
			MPI_Recv(tmpMsg, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
			if (this->myState == SEARCHING_FOR_PUB || this->myState == IN_PUB) {
				// remember about priority
				if (this->myPub != tmpMsg[1]) {
					tmpMsg[0] = clk;
					tmpMsg[1] = this->myPub;
					std::cout << "Worker " << this->rank << " accepting request from " << status->MPI_SOURCE << std::endl;
					MPI_Send(tmpMsg, 2, MPI_INT, status->MPI_SOURCE, ACCEPT, MPI_COMM_WORLD); 
				}
				else {
					waitingForAccept.push(status->MPI_SOURCE);
				}
			} else {
				tmpMsg[0] = clk;
				tmpMsg[1] = NOT_IN_PUB;
				std::cout << "Worker " << this->rank << " accepting request from " << status->MPI_SOURCE << std::endl;
				MPI_Send(tmpMsg, 2, MPI_INT, status->MPI_SOURCE, ACCEPT, MPI_COMM_WORLD);
			}
		}
		break;
		default: {
			int tmpMsg;
			MPI_Recv(&tmpMsg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
		}
	}
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
			int processIter;

			while (alcoholics.size() > 0) {
				processIter = rand() % alcoholics.size();
				msg = this->clk;
				MPI_Send(&msg, 1, MPI_INT, alcoholics[processIter], WANNA_DRINK, MPI_COMM_WORLD);
				std::cout << "Worker " << this->rank<< " : waiting for message from " << alcoholics[processIter] << std::endl;
				this->waitForMessageFrom(alcoholics[processIter]);
				this->dispatchMessage(&status, NULL);
				if (status.MPI_TAG == SURE) {
					std::cout << "Worker " << this->rank << " : waiting in queue with " << alcoholics[processIter] << std::endl;
					this->partnerRank = status.MPI_SOURCE;
					this->myState = SEARCHING_FOR_PUB;
					break;
				}
				alcoholics.erase(alcoholics.begin() + processIter);
			}
		break;
		}
		case SEARCHING_FOR_PUB: {
			this->myPub = rand() % Utils::settings.pubCount;
			int msg[2] = {this->clk, this->myPub};
			int flag,acceptCount = 0;

			for (int processIter = 0; processIter < this->workerCount; ++processIter) {
				if (processIter != this->rank) {
					//std::cout << "Worker " << this->rank << " : sending request (Pub " << this->myPub << ")" << std::endl;
					MPI_Send(msg, 2, MPI_INT, processIter, CAN_ENTER, MPI_COMM_WORLD);
				}
			}

			int waitCount = this->workerCount;
			int acceptsRequired = this->workerCount - Utils::settings.pubCapacity[this->myPub];
			MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
			while (waitCount > 0 && acceptsRequired > 0) {
				if (flag) {
					this->dispatchMessage(&status, msg);
					if (status.MPI_TAG == ACCEPT) {
						if (++acceptCount > acceptsRequired) {
							break;
						}
						if (msg[1] != NOT_IN_PUB) this->pubCapacities[msg[1]]++;
						--waitCount;
					}
				}
				MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
				Utils::msleep(SLEEP_TIME);
			}
			std::cout << "Worker " << this->rank << " enters pub " << this->myPub << std::endl;
			this->myState = IN_PUB;
			this->remainDrinkTime = MIN_PUB_TIME + rand() % TOLERANCE_PUB_TIME;
		}
		break;
		case IN_PUB: {
			if (this->remainDrinkTime <= 0) {
				// informing others that they can leave
				std::cout << "Worker " << this->rank << " and Alcoholic " << this->partnerRank << " are leaving pub " << this->myPub << std::endl;
				this->myPub = NOT_IN_PUB;
				int msg[2] = {this->clk, this->myPub};
				while (!waitingForAccept.empty()) {
					MPI_Send(msg, 2, MPI_INT, waitingForAccept.top(), ACCEPT, MPI_COMM_WORLD);
					waitingForAccept.pop();
				}
				MPI_Send(&this->clk, 1, MPI_INT, this->partnerRank, NO_MORE_DRINKING, MPI_COMM_WORLD);
				this->partnerRank = NO_PARTNER;
				this->myState = SEARCHING_FOR_PAIR;
			}
			this->remainDrinkTime -= SLEEP_TIME;
		break;
		}
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
			this->dispatchMessage(&status, NULL);
		}
		MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
		Utils::msleep(SLEEP_TIME);
	}
}

Alcoholic::Alcoholic() {
	this->myState = WAITING_FOR_PAIR;
}

Alcoholic::~Alcoholic() {
	
}

void Alcoholic::dispatchMessage(MPI_Status * status, int * msg) {
	int tmpMsg;
	switch (status->MPI_TAG) {
		case WANNA_DRINK:
			MPI_Recv(&tmpMsg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
			if (this->myState == WAITING_FOR_PAIR) {
				this->myState = IN_PAIR;
				std::cout << "Alcoholic " << this->rank << " agreed to " << status->MPI_SOURCE << std::endl;
				MPI_Send(&this->clk, 1, MPI_INT, status->MPI_SOURCE, SURE, MPI_COMM_WORLD); 
			}
			else {
				std::cout << "Alcoholic " << this->rank << " disagreed to " << status->MPI_SOURCE << std::endl;
				MPI_Send(&this->clk, 1, MPI_INT, status->MPI_SOURCE, NOPE, MPI_COMM_WORLD); 
			}
		break;
		
		case NO_MORE_DRINKING:
			MPI_Recv(&tmpMsg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
			if (this->myState == IN_PAIR) {
				this->myState = WAITING_FOR_SOBER_STATION;
			}
		break;
		
		case CAN_ENTER_SOBER_STATION:
			std::cout << "Alcoholic " << this->rank << " got request from " << status->MPI_SOURCE << std::endl;
			MPI_Recv(&tmpMsg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
			if (this->myState == WAITING_FOR_SOBER_STATION || this->myState == IN_SOBER_STATION) {
				waitingForAccept.push(status->MPI_SOURCE);
			} else {
				MPI_Send(&this->clk, 1, MPI_INT, status->MPI_SOURCE, ACCEPT, MPI_COMM_WORLD);
			}
		break;

		default: {
			MPI_Recv(&tmpMsg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
		}
	}
}

Alcoholic & Alcoholic::getInstance() {
	static Alcoholic instance;
	return instance;
}

void Alcoholic::performAction() {
	MPI_Status status;
	switch (this->myState){
		case WAITING_FOR_SOBER_STATION: {
			int flag, msg, acceptCount = 0;
			msg = this->clk;
			for (int processIter = this->workerCount; processIter < size; ++processIter) {
				if (processIter != this->rank) {
					MPI_Send(&msg, 1, MPI_INT, processIter, CAN_ENTER_SOBER_STATION, MPI_COMM_WORLD);
				}
			}

			int waitCount = (this->size - this->workerCount);
			int acceptsRequired = (this->size - this->workerCount) - Utils::settings.soberStationCapacity;
			MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
			while (waitCount > 0 && acceptsRequired > 0) {
				if (flag) {
					this->dispatchMessage(&status, &msg);
					if (status.MPI_TAG == ACCEPT) {
						if (++acceptCount > acceptsRequired) {
							break;
						}
						--waitCount;
					}
				}
				MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
				Utils::msleep(SLEEP_TIME);
			}
			std::cout << "Alcoholic " << this->rank << " enters sober station" << std::endl;
			this->myState = IN_SOBER_STATION;
			this->remainRestTime = MIN_REST_TIME + rand() % TOLERANCE_REST_TIME;
						
		break; 
		}

		case IN_SOBER_STATION: {
			if (this->remainRestTime <= 0) {
				std::cout << "Alcoholic " << this->rank << " leaving sober station" << std::endl;
				while (!waitingForAccept.empty()) {
					MPI_Send(&this->clk, 1, MPI_INT, waitingForAccept.top(), ACCEPT, MPI_COMM_WORLD);
					waitingForAccept.pop();
				}
				this->myState = WAITING_FOR_PAIR;
			}
			this->remainRestTime -= SLEEP_TIME;
		break;	
		}
	}
}

void Alcoholic::showIdentity() {
	std::cout << "I'm an alcoholic" << std::endl;
}