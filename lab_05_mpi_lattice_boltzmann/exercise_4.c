/*****************************************************
    AUTHOR  : Sébastien Valat
    MAIL    : sebastien.valat@univ-grenoble-alpes.fr
    LICENSE : BSD
    YEAR    : 2021
    COURSE  : Parallel Algorithms and Programming
*****************************************************/

//////////////////////////////////////////////////////
//
// Goal: Implement 2D grid communication scheme with
//       8 neighbors using manual copy for non
//       contiguous side and blocking communications
//
// SUMMARY:
//     - 2D splitting along X and Y
//     - 8 neighbors communications
//     - Blocking communications
//     - Manual copy for non continguous cells
//
//////////////////////////////////////////////////////

/****************************************************/
#include "src/lbm_struct.h"
#include "src/exercises.h"

/****************************************************/
void lbm_comm_init_ex4(lbm_comm_t * comm, int total_width, int total_height)
{
	int rank;
	int comm_size;
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &comm_size );

	if(comm_size % 2 != 0 ){
		fatal("Invalid communicator size, dimension invalid");
	}
	// TODO: calculate the number of tasks along X axis and Y axis.
	comm->nb_x = comm_size / 2;
	comm->nb_y = 2;

	if(total_width % comm->nb_x != 0 || total_height % comm->nb_y != 0){
		fatal("Invalid communicator size, dimension invalid");
	}

	comm->rank_x = rank % comm->nb_x;
	comm->rank_y = rank / comm->nb_x;

	// calculate the local sub-domain size
	comm->width = total_width/comm->nb_x + 2;
	comm->height = total_height/comm->nb_y + 2;

	// calculate the absolute position in the global mesh.
	//        without accounting the ghost cells
	//        (used to setup the obstable & initial conditions).
	comm->x = (total_width/comm->nb_x)*comm->rank_x;
	comm->y = (total_height/comm->nb_y)*comm->rank_y;

	size_t row_size = comm->width * DIRECTIONS * sizeof(double);
	comm->buffer_recv_down = (double*) malloc(row_size);
	comm->buffer_recv_up = (double*) malloc(row_size);
	comm->buffer_send_down = (double*) malloc(row_size);
	comm->buffer_send_up = (double*) malloc(row_size);

	//if debug print comm
	//lbm_comm_print(comm);
	#ifndef NDEBUG
	lbm_comm_print( comm );
	#endif
}

/****************************************************/
void lbm_comm_release_ex4(lbm_comm_t * comm)
{
	//free allocated ressources
	free(comm->buffer_recv_down);
	free(comm->buffer_recv_up);
	free(comm->buffer_send_down);
	free(comm->buffer_send_up);
}

int rank_find(int x, int y, int width, int height) {
	return y * width + x;
} 

/****************************************************/
void lbm_comm_ghost_exchange_ex4(lbm_comm_t * comm, lbm_mesh_t * mesh)
{
    int rank_up = MPI_PROC_NULL;
    int rank_down = MPI_PROC_NULL;
    int rank_left = MPI_PROC_NULL;
    int rank_right = MPI_PROC_NULL;
    int rank_tr = MPI_PROC_NULL;
    int rank_tl = MPI_PROC_NULL;
    int rank_br = MPI_PROC_NULL;
    int rank_bl = MPI_PROC_NULL;

    if (comm->rank_y > 0) {
		rank_up = rank_find(comm->rank_x, comm->rank_y - 1, comm->nb_x, comm->nb_y);
	}
    if (comm->rank_y < comm->nb_y - 1) {
		rank_down = rank_find(comm->rank_x, comm->rank_y + 1, comm->nb_x, comm->nb_y);
	}
    if (comm->rank_x > 0) {
		rank_left = rank_find(comm->rank_x - 1, comm->rank_y, comm->nb_x, comm->nb_y);
	}
    if (comm->rank_x < comm->nb_x - 1) {
		rank_right = rank_find(comm->rank_x + 1, comm->rank_y, comm->nb_x, comm->nb_y);
	}

    if (comm->rank_x > 0 && comm->rank_y > 0) { 
		rank_tl = rank_find(comm->rank_x - 1, comm->rank_y - 1, comm->nb_x, comm->nb_y); 
	}
    if (comm->rank_x < comm->nb_x - 1 && comm->rank_y > 0) { 
		rank_tr = rank_find(comm->rank_x + 1, comm->rank_y - 1, comm->nb_x, comm->nb_y); 
	}
    if (comm->rank_x > 0 && comm->rank_y < comm->nb_y - 1) { 
		rank_bl = rank_find(comm->rank_x - 1, comm->rank_y + 1, comm->nb_x, comm->nb_y); 
	}
    if (comm->rank_x < comm->nb_x - 1 && comm->rank_y < comm->nb_y - 1) { 
		rank_br = rank_find(comm->rank_x + 1, comm->rank_y + 1, comm->nb_x, comm->nb_y); 
	}

    // left and right send recv
    for (int i = 0; i < comm->height; i++) {
        MPI_Sendrecv(lbm_mesh_get_cell(mesh, 1, i), DIRECTIONS, MPI_DOUBLE, rank_left, 0,
                     lbm_mesh_get_cell(mesh, comm->width - 1, i), DIRECTIONS, MPI_DOUBLE, rank_right, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Sendrecv(lbm_mesh_get_cell(mesh, comm->width - 2, i), DIRECTIONS, MPI_DOUBLE, rank_right, 1,
                     lbm_mesh_get_cell(mesh, 0, i), DIRECTIONS, MPI_DOUBLE, rank_left, 1,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // up and down packing
    for (int i = 0; i < comm->width; i++) {
        double* top = lbm_mesh_get_cell(mesh, i, 1);
        double* bot = lbm_mesh_get_cell(mesh, i, comm->height - 2);
        for (int d = 0; d < DIRECTIONS; d++) {
            comm->buffer_send_up[i * DIRECTIONS + d] = top[d];
            comm->buffer_send_down[i * DIRECTIONS + d] = bot[d];
        }
    }
	//then send recv up and down
    MPI_Sendrecv(comm->buffer_send_up, comm->width * DIRECTIONS, MPI_DOUBLE, rank_up, 2,
                 comm->buffer_recv_down, comm->width * DIRECTIONS, MPI_DOUBLE, rank_down, 2,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    MPI_Sendrecv(comm->buffer_send_down, comm->width * DIRECTIONS, MPI_DOUBLE, rank_down, 3,
                 comm->buffer_recv_up, comm->width * DIRECTIONS, MPI_DOUBLE, rank_up, 3,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);


	//unpacking
    for (int i = 0; i < comm->width; i++) {
        double* top_ghost = lbm_mesh_get_cell(mesh, i, 0);
        double* bot_ghost = lbm_mesh_get_cell(mesh, i, comm->height - 1);
        for (int d = 0; d < DIRECTIONS; d++) {
            if (rank_up != MPI_PROC_NULL) top_ghost[d] = comm->buffer_recv_up[i * DIRECTIONS + d];
            if (rank_down != MPI_PROC_NULL) bot_ghost[d] = comm->buffer_recv_down[i * DIRECTIONS + d];
        }
    }

    // diagonals
    MPI_Sendrecv(lbm_mesh_get_cell(mesh, 1, 1), DIRECTIONS, MPI_DOUBLE, rank_tl, 4,
                 lbm_mesh_get_cell(mesh, comm->width - 1, comm->height - 1), DIRECTIONS, MPI_DOUBLE, rank_br, 4,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    MPI_Sendrecv(lbm_mesh_get_cell(mesh, comm->width - 2, 1), DIRECTIONS, MPI_DOUBLE, rank_tr, 5,
                 lbm_mesh_get_cell(mesh, 0, comm->height - 1), DIRECTIONS, MPI_DOUBLE, rank_bl, 5,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    MPI_Sendrecv(lbm_mesh_get_cell(mesh, 1, comm->height - 2), DIRECTIONS, MPI_DOUBLE, rank_bl, 6,
                 lbm_mesh_get_cell(mesh, comm->width - 1, 0), DIRECTIONS, MPI_DOUBLE, rank_tr, 6,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    MPI_Sendrecv(lbm_mesh_get_cell(mesh, comm->width - 2, comm->height - 2), DIRECTIONS, MPI_DOUBLE, rank_br, 7,
                 lbm_mesh_get_cell(mesh, 0, 0), DIRECTIONS, MPI_DOUBLE, rank_tl, 7,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}