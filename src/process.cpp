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
#include <sstream>


void Process::dispatchMessage(MPI_Status * status, int * msg) {
}

int Process::run(int rank, int size, int val) {

	int flag;
	this->workerCount = val;
	this->size = size;
	this->rank = rank;
	std::stringstream ss;
	ss << "log/sw_" << rank << ".log";
	this->logFile.open(ss.str().c_str(), std::fstream::out | std::fstream::trunc);
	MPI_Status status;
	srand(time(NULL));

	while (this->cycles < Utils::settings.iterations) {
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

void Process::updateClock(int otherClk) {
	if (this->clk > otherClk) this->clk++;
	else this->clk = otherClk + 1;
}

SocialWorker::SocialWorker() {
	this->myPub = NOT_IN_PUB;
	this->partnerRank = NO_PARTNER;
	this->myState = SEARCHING_FOR_PAIR;
	this->clk = 0;
	this->pubQueues = new int [Utils::settings.pubCount];
	for (int i = 0; i < Utils::settings.pubCount; ++i) {
		this->pubQueues[i] = 0;
	}
}

SocialWorker::~SocialWorker() {
	delete [] this->pubQueues;
}

void SocialWorker::dispatchMessage(MPI_Status * status, int * msg) {
	switch(status->MPI_TAG) {
		case ACCEPT: {
			if (!msg) {
				int tmpMsg[3];
				MPI_Recv(tmpMsg, 3, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
				this->updateClock(tmpMsg[0]);			
			}
			else {
				MPI_Recv(msg, 3, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
				this->updateClock(msg[0]);
			}
		}
		break;
		case CAN_ENTER: {
			this->logFile << "Worker " << this->rank << " got request from " << status->MPI_SOURCE << std::endl;
			int tmpMsg[3];
			MPI_Recv(tmpMsg, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
			this->updateClock(tmpMsg[0]);
			if (this->myState == SEARCHING_FOR_PUB || this->myState == IN_PUB) {
				// remember about prio
				if (this->myPub != tmpMsg[1]) {
					tmpMsg[0] = this->clk;
					tmpMsg[2] = tmpMsg[1];
					tmpMsg[1] = this->myPub;
					this->logFile << "Worker " << this->rank << " accepting request from " << status->MPI_SOURCE << std::endl;
					MPI_Send(tmpMsg, 3, MPI_INT, status->MPI_SOURCE, ACCEPT, MPI_COMM_WORLD); 
				}
				else if (this->myState == SEARCHING_FOR_PUB && tmpMsg[0] <= this->clk) {
					if (tmpMsg[0] < this->clk || status->MPI_SOURCE < this->rank) {
						tmpMsg[0] = this->clk;
						tmpMsg[2] = tmpMsg[1];
						tmpMsg[1] = this->myPub;
						this->logFile << "Worker " << this->rank << " accepting request from " << status->MPI_SOURCE << std::endl;
						MPI_Send(tmpMsg, 3, MPI_INT, status->MPI_SOURCE, ACCEPT, MPI_COMM_WORLD); 
					}	
				}
				else {
					waitingForAccept.push(status->MPI_SOURCE);
				}
			} else {
				tmpMsg[0] = this->clk;
				tmpMsg[2] = tmpMsg[1];
				tmpMsg[1] = NOT_IN_PUB;
				this->logFile << "Worker " << this->rank << " accepting request from " << status->MPI_SOURCE << std::endl;
				MPI_Send(tmpMsg, 3, MPI_INT, status->MPI_SOURCE, ACCEPT, MPI_COMM_WORLD);
			}
		}
		break;
		default: {
			int tmpMsg;
			MPI_Recv(&tmpMsg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
			this->updateClock(tmpMsg);
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
			this->logFile << "Worker " <<this->rank << " searching for pair, " <<this->workerCount << " " << this->size << std::endl;
			int processIter;

			while (alcoholics.size() > 0) {
				processIter = rand() % alcoholics.size();
				msg = this->clk;
				MPI_Send(&msg, 1, MPI_INT, alcoholics[processIter], WANNA_DRINK, MPI_COMM_WORLD);
				this->logFile << "Worker " << this->rank<< " waiting for message from " << alcoholics[processIter] << std::endl;
				this->waitForMessageFrom(alcoholics[processIter]);
				this->dispatchMessage(&status, NULL);
				if (status.MPI_TAG == SURE) {
					this->logFile << "Worker " << this->rank << " waiting in queue with " << alcoholics[processIter] << std::endl;
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
			int msg[3] = {this->clk, this->myPub, this->myPub};
			int flag,acceptCount = 0;

			for (int processIter = 0; processIter < this->workerCount; ++processIter) {
				if (processIter != this->rank) {
					//this->logFile << "Worker " << this->rank << " : sending request (Pub " << this->myPub << ")" << std::endl;
					MPI_Send(msg, 2, MPI_INT, processIter, CAN_ENTER, MPI_COMM_WORLD);
				}
			}

			int iterationLimit = 3;
			int waitCount = this->workerCount - 1;
			int acceptsRequired = waitCount - Utils::settings.pubCapacity[this->myPub];
			MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
			while (waitCount > 0 && acceptsRequired > 0) {
				if (flag) {
					this->dispatchMessage(&status, msg);
					if (status.MPI_TAG == ACCEPT) {
						//ACCEPT must concern pub that process is waiting for
						if (msg[2] == this->myPub) {
							if (++acceptCount > acceptsRequired) {
								break;
							}
						
						if (msg[1] != NOT_IN_PUB) this->pubQueues[msg[1]]++;
						--waitCount;
						}
					}
				}
				MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
				Utils::msleep(SLEEP_TIME);
				if (--iterationLimit == 0) {
					int bestPub = this->myPub;
					int bestQueueLength = waitCount - Utils::settings.pubCapacity[this->myPub];
					iterationLimit = 3;
					for (int i = 0; i < Utils::settings.pubCount; ++i) {
						if (i != this->myPub) {
							int tempQueueLength = this->pubQueues[i] - Utils::settings.pubCapacity[i];
							if (tempQueueLength < 0) {
								bestPub = i;
								bestQueueLength = tempQueueLength;
								break;
							} else if (tempQueueLength < bestQueueLength) {
								bestPub = i;
								bestQueueLength = tempQueueLength;
							}
						}
					}
					if (bestQueueLength < 0 || (bestQueueLength + int(0.05 * this->workerCount) + 2) < (waitCount - Utils::settings.pubCapacity[this->myPub])) {
						this->myPub = bestPub;
						msg[0] = this->clk;
						msg[1] = this->myPub;
						while (!waitingForAccept.empty()) {
							MPI_Send(msg, 3, MPI_INT, waitingForAccept.top(), ACCEPT, MPI_COMM_WORLD);
							waitingForAccept.pop();
						}
						waitCount = this->workerCount - 1;
						acceptsRequired = waitCount - Utils::settings.pubCapacity[this->myPub];
						for (int i = 0; i < Utils::settings.pubCount; ++i) {
							this->pubQueues[i] = 0;
						}
					}

				}
			}
			//cleaning after state
			for (int i = 0; i < Utils::settings.pubCount; ++i) {
				this->pubQueues[i] = 0;
			}
			this->logFile << "Worker " << this->rank << " enters pub " << this->myPub << std::endl;
			this->myState = IN_PUB;
			this->remainDrinkTime = MIN_PUB_TIME + rand() % TOLERANCE_PUB_TIME;
		}
		break;
		case IN_PUB: {
			if (this->remainDrinkTime <= 0) {
				// informing others that they can leave
				this->logFile << "Worker " << this->rank << " and Alcoholic " << this->partnerRank << " are leaving pub " << this->myPub << std::endl;
				this->myPub = NOT_IN_PUB;
				int msg[3] = {this->clk, this->myPub, this->myPub};
				while (!waitingForAccept.empty()) {
					MPI_Send(msg, 3, MPI_INT, waitingForAccept.top(), ACCEPT, MPI_COMM_WORLD);
					waitingForAccept.pop();
				}
				MPI_Send(&this->clk, 1, MPI_INT, this->partnerRank, NO_MORE_DRINKING, MPI_COMM_WORLD);
				this->partnerRank = NO_PARTNER;
				this->myState = SEARCHING_FOR_PAIR;
				this->cycles++;
			}
			this->remainDrinkTime -= SLEEP_TIME;
		break;
		}
	}

}

void SocialWorker::showIdentity() {
	this->logFile << "I'm a social worker" << std::endl;
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
	this->clk = 0;
}

Alcoholic::~Alcoholic() {
	
}

void Alcoholic::dispatchMessage(MPI_Status * status, int * msg) {
	int tmpMsg;
	switch (status->MPI_TAG) {
		case WANNA_DRINK:
			MPI_Recv(&tmpMsg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
			this->updateClock(tmpMsg);
			if (this->myState == WAITING_FOR_PAIR) {
				this->myState = IN_PAIR;
				this->logFile << "Alcoholic " << this->rank << " agreed to " << status->MPI_SOURCE << std::endl;
				MPI_Send(&this->clk, 1, MPI_INT, status->MPI_SOURCE, SURE, MPI_COMM_WORLD); 
			}
			else {
				this->logFile << "Alcoholic " << this->rank << " disagreed to " << status->MPI_SOURCE << std::endl;
				MPI_Send(&this->clk, 1, MPI_INT, status->MPI_SOURCE, NOPE, MPI_COMM_WORLD); 
			}
		break;
		
		case NO_MORE_DRINKING:
			MPI_Recv(&tmpMsg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
			this->updateClock(tmpMsg);
			if (this->myState == IN_PAIR) {
				this->myState = WAITING_FOR_SOBER_STATION;
			}
		break;
		
		case CAN_ENTER_SOBER_STATION:
			this->logFile << "Alcoholic " << this->rank << " got request from " << status->MPI_SOURCE << std::endl;
			MPI_Recv(&tmpMsg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
			this->updateClock(tmpMsg);
			if (this->myState == WAITING_FOR_SOBER_STATION || this->myState == IN_SOBER_STATION) {
				if (this->myState == WAITING_FOR_SOBER_STATION && tmpMsg <= this->clk) {
					if (tmpMsg < this->clk || status->MPI_SOURCE < this->rank) {
						tmpMsg = this->clk;
						MPI_Send(&this->clk, 1, MPI_INT, status->MPI_SOURCE, ACCEPT, MPI_COMM_WORLD);
					}
				}
				else waitingForAccept.push(status->MPI_SOURCE);
			} else {
				MPI_Send(&this->clk, 1, MPI_INT, status->MPI_SOURCE, ACCEPT, MPI_COMM_WORLD);
			}
		break;

		default: {
			MPI_Recv(&tmpMsg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
			this->updateClock(tmpMsg);		
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
			this->logFile << "Alcoholic " << this->rank << " enters sober station" << std::endl;
			this->myState = IN_SOBER_STATION;
			this->remainRestTime = MIN_REST_TIME + rand() % TOLERANCE_REST_TIME;
						
		break; 
		}

		case IN_SOBER_STATION: {
			if (this->remainRestTime <= 0) {
				this->logFile << "Alcoholic " << this->rank << " leaving sober station" << std::endl;
				while (!waitingForAccept.empty()) {
					MPI_Send(&this->clk, 1, MPI_INT, waitingForAccept.top(), ACCEPT, MPI_COMM_WORLD);
					waitingForAccept.pop();
				}
				this->myState = WAITING_FOR_PAIR;
				this->cycles++;
			}
			this->remainRestTime -= SLEEP_TIME;
		break;	
		}
	}
}

void Alcoholic::showIdentity() {
	this->logFile << "I'm an alcoholic" << std::endl;
}