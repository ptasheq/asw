#include <mpi.h>
#include "process.h"
#include "utils.h"

int main(int argc, char **argv) {

	MPI_Init(&argc,&argv);
	int size, rank, len;

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int val = Utils::checkArguments(argc, argv);
	Utils::loadSettings();

	if (size <= val) {
		MPI_Finalize();
		return 0;
	}
	return (rank < val) ? SocialWorker::getInstance().run(rank, size, val) : Alcoholic::getInstance().run(rank, size, val);
}
