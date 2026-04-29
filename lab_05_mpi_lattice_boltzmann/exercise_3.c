/*****************************************************
    AUTHOR  : Sébastien Valat
    MAIL    : sebastien.valat@univ-grenoble-alpes.fr
    LICENSE : BSD
    YEAR    : 2021
    COURSE  : Parallel Algorithms and Programming
*****************************************************/

//////////////////////////////////////////////////////
//
// Goal: Implement non-blocking 1D communication scheme
//       along X axis.
//
// SUMMARY:
//     - 1D splitting along X
// NEW:
//     - >>> Non-blocking communications <<<
//
//////////////////////////////////////////////////////

/****************************************************/
#include "src/lbm_struct.h"
#include "src/exercises.h"

/****************************************************/
void lbm_comm_init_ex3(lbm_comm_t * comm, int total_width, int total_height)
{
	//we use the same implementation then ex1
	lbm_comm_init_ex1(comm, total_width, total_height);
}

/****************************************************/
void lbm_comm_ghost_exchange_ex3(lbm_comm_t * comm, lbm_mesh_t * mesh)
{
	//check if not in leftmost pos
	for(int i = 0; i < comm->height; i++){
		MPI_Request requests[4] = {
			MPI_REQUEST_NULL,
			MPI_REQUEST_NULL,
			MPI_REQUEST_NULL,
			MPI_REQUEST_NULL
		};

		// send left
		if(comm->rank_x !=0){
			double * cell = lbm_mesh_get_cell(mesh, 1, i);
			MPI_Isend((void*)cell, DIRECTIONS, MPI_DOUBLE, comm->rank_x-1,0,MPI_COMM_WORLD, &requests[0]);
		} 
		// receive right
		if(comm->rank_x != comm->nb_x-1){
			double * cell = lbm_mesh_get_cell(mesh, comm->width-1, i);
			MPI_Irecv((void*)cell, DIRECTIONS, MPI_DOUBLE, comm->rank_x+1,0,MPI_COMM_WORLD, &requests[1]);
		}
		// send right
		if(comm->rank_x != comm->nb_x-1){
			double * cell = lbm_mesh_get_cell(mesh, comm->width-2, i);
			MPI_Isend((void*)cell, DIRECTIONS, MPI_DOUBLE, comm->rank_x+1,1,MPI_COMM_WORLD, &requests[2]);
		}
		// receive left
		if(comm->rank_x !=0){
			double * cell = lbm_mesh_get_cell(mesh, 0, i);
			MPI_Irecv((void*)cell, DIRECTIONS, MPI_DOUBLE, comm->rank_x-1,1,MPI_COMM_WORLD, &requests[3]);
		}
		MPI_Waitall(4, requests, MPI_STATUSES_IGNORE);
	}
}
