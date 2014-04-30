#include "process.h"
#include "protocol.h"
#include "utils.h"
#include <iostream>
#include <mpi.h>
#include <unistd.h>

void Process::dispatchMessage(MPI_Status * status) {
}

int Process::run(int rank, int size) {

	Utils::loadSettings(this->settings);

	int flag;
	MPI_Status status;
	while (true) {
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
	MPI_Send(&this->rank, 1, MPI_INT, 1, WANNA_DRINK, MPI_COMM_WORLD);
	std::cout << "message sent" << std::endl;
}

void SocialWorker::showIdentity() {
	std::cout << "I'm a social worker" << std::endl;
}

Alcoholic::Alcoholic() {

}

Alcoholic::~Alcoholic() {
	
}

void Alcoholic::dispatchMessage(MPI_Status * status) {
	MPI_Recv(&msg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
	std::cout << "message received" << std::endl;
	MPI_Send(&this->rank, 1, MPI_INT, status->MPI_SOURCE, SURE, MPI_COMM_WORLD);
}

Alcoholic & Alcoholic::getInstance() {
	static Alcoholic instance;
	return instance;
}

void Alcoholic::performAction() {
}

void Alcoholic::showIdentity() {
	std::cout << "I'm an alcoholic" << std::endl;
}