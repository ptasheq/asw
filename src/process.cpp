#include "process.h"
#include <iostream>
#include <mpi.h>

int Process::run(int rank, int size) {
	MPI_Finalize();
	return 0;
}

SocialWorker::SocialWorker() {

}

SocialWorker::~SocialWorker() {

}

SocialWorker & SocialWorker::getInstance() {
	static SocialWorker instance;
	return instance;
}

int SocialWorker::run(int rank, int size) {
	std::cout << "I'm a social worker" << std::endl;
	MPI_Finalize();
	return 0;
}

Alcoholic::Alcoholic() {

}

Alcoholic::~Alcoholic() {
	
}

Alcoholic & Alcoholic::getInstance() {
	static Alcoholic instance;
	return instance;
}

int Alcoholic::run(int rank, int size) {
	std::cout << "I'm an alcoholic" << std::endl;
	MPI_Finalize();
	return 0;
}