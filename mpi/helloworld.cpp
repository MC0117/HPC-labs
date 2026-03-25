#include <iostream>
#include <mpi.h>

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank % 2) {
        std::cout << "MPI rank odd: " << rank <<
        std::endl;
    } else {
        std::cout << "MPI rank even: " << rank <<
     std::endl;
    }

    // int size;
    // MPI_Comm_size(MPI_COMM_WORLD, &size);
    // std::cout << "MPI size: " << size <<
    // std::endl;

    MPI_Finalize();
    return 0;
}