#include <iostream>
#include <mpi.h>

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    MPI_Comm mpi_comm = MPI_COMM_WORLD;

    int rank, size;
    MPI_Comm_rank(mpi_comm, &rank);
    MPI_Comm_size(mpi_comm, &size);

    double start = MPI_Wtime();

    if (rank == 0) {
        double buf_send[10] = {5.0};
        std::cout << "sending data " ;
        for (int i = 10 - 1; i >= 0; i--) {
            std::cout << buf_send[i];
        } 
        std::cout << std::endl;
        MPI_Send(&buf_send, 10, MPI_DOUBLE, rank+1, 0, mpi_comm);

        double buf_recv[10]; 
        MPI_Recv(&buf_recv, 10, MPI_DOUBLE, size-1, 0, mpi_comm, 0);

        std::cout << "Rank " << rank; 
        std::cout << " rcvd data ";
        for (int i = 10 - 1; i >= 0; i--) {
            std::cout << buf_recv[i];
        }
        std::cout << std::endl;

    } else {
        double buf_recv[10]; 
        MPI_Recv(&buf_recv, 10, MPI_DOUBLE, rank-1, 0, mpi_comm, 0);
        for (int i = 10 - 1; i >= 0; i--) {
            buf_recv[i] = buf_recv[i] + 1;
        }

        MPI_Send(&buf_recv, 10, MPI_DOUBLE, (rank+1)%size, 0, mpi_comm);

    }

    double end = MPI_Wtime();

    std::cout << "Total time for rank " << rank << ": " << end - start << std::endl;

    MPI_Finalize();
    return 0;
}