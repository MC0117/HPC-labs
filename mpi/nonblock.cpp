#include <iostream>
#include <mpi.h>

#define M_SIZE 10

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    MPI_Comm mpi_comm = MPI_COMM_WORLD;

    int rank, size;
    MPI_Comm_rank(mpi_comm, &rank);
    MPI_Comm_size(mpi_comm, &size);

    int request_count = 0;
    MPI_Request requests[2];

    double buf_recv[10]; 

    double start = MPI_Wtime();

    if (rank == 0) {
        double buf_send[M_SIZE] = {5.0};
        std::cout << "sending data " ;
        for (int i = M_SIZE - 1; i >= 0; i--) {
            std::cout << buf_send[i];
        } 
        std::cout << std::endl;
        MPI_Isend(&buf_send, M_SIZE, MPI_DOUBLE, rank+1, 0, mpi_comm, &requests[request_count++]);

        MPI_Irecv(&buf_recv, M_SIZE, MPI_DOUBLE, size-1, 1, mpi_comm, &requests[request_count++]);

    } else {
        MPI_Irecv(&buf_recv, M_SIZE, MPI_DOUBLE, rank-1, 0, mpi_comm, &requests[request_count++]);

        double buf_send[M_SIZE] = {6.0};
        // for (int i = M_SIZE - 1; i >= 0; i--) {
        //     buf_send[i] = buf_recv[i]++;
        // } 
        MPI_Isend(&buf_send, M_SIZE, MPI_DOUBLE, (rank+1)%size, 1, mpi_comm, &requests[request_count++]);
    }
    MPI_Waitall(request_count, requests, MPI_STATUSES_IGNORE);

    double end = MPI_Wtime();

    std::cout << "Rank " << rank; 
    std::cout << " rcvd data ";
    for (int i = M_SIZE - 1; i >= 0; i--) {
        std::cout << buf_recv[i];
    }
    std::cout << std::endl;

    std::cout << "Total time for rank " << rank << ": " << end - start << std::endl;

    MPI_Finalize();
    return 0;
}