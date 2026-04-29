/*****************************************************
    AUTHOR  : Sébastien Valat
    MAIL    : sebastien.valat@univ-grenoble-alpes.fr
    LICENSE : BSD
    YEAR    : 2021
    COURSE  : Parallel Algorithms and Programming
*****************************************************/

//////////////////////////////////////////////////////
//
// Goal: Implement odd/even 1D blocking communication scheme 
//       along X axis.
//
// SUMMARY:
//     - 1D splitting along X
//     - Blocking communications
// NEW:
//     - >>> Odd/even communication ordering <<<<
//
//////////////////////////////////////////////////////

/****************************************************/
#include "src/lbm_struct.h"
#include "src/exercises.h"

/****************************************************/
void lbm_comm_init_ex2(lbm_comm_t * comm, int total_width, int total_height)
{
	//we use the same implementation then ex1
	lbm_comm_init_ex1(comm, total_width, total_height);
}

/****************************************************/
void lbm_comm_ghost_exchange_ex2(lbm_comm_t * comm, lbm_mesh_t * mesh)
{
	// odd -> send first
	if (comm->rank_x % 2) {
		//check if not in leftmost pos
		for(int i = 0; i < comm->height; i++){
			// send left
			if(comm->rank_x !=0){
				double * cell = lbm_mesh_get_cell(mesh, 1, i);
				MPI_Send((void*)cell, DIRECTIONS, MPI_DOUBLE, comm->rank_x-1,0,MPI_COMM_WORLD);
			} 
			// receive right
			if(comm->rank_x != comm->nb_x-1){
				double * cell = lbm_mesh_get_cell(mesh, comm->width-1, i);
				MPI_Recv((void*)cell, DIRECTIONS, MPI_DOUBLE, comm->rank_x+1,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
			// send right
			if(comm->rank_x != comm->nb_x-1){
				double * cell = lbm_mesh_get_cell(mesh, comm->width-2, i);
				MPI_Send((void*)cell, DIRECTIONS, MPI_DOUBLE, comm->rank_x+1,1,MPI_COMM_WORLD);
			}
			// receive left
			if(comm->rank_x !=0){
				double * cell = lbm_mesh_get_cell(mesh, 0, i);
				MPI_Recv((void*)cell, DIRECTIONS, MPI_DOUBLE, comm->rank_x-1,1,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
		}
	} else {
		// even -> receive first
		for(int i = 0; i < comm->height; i++){
			// receive right
			if(comm->rank_x != comm->nb_x-1){
				double * cell = lbm_mesh_get_cell(mesh, comm->width-1, i);
				MPI_Recv((void*)cell, DIRECTIONS, MPI_DOUBLE, comm->rank_x+1,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
			// send left
			if(comm->rank_x !=0){
				double * cell = lbm_mesh_get_cell(mesh, 1, i);
				MPI_Send((void*)cell, DIRECTIONS, MPI_DOUBLE, comm->rank_x-1,0,MPI_COMM_WORLD);
			} 
			// receive left
			if(comm->rank_x !=0){
				double * cell = lbm_mesh_get_cell(mesh, 0, i);
				MPI_Recv((void*)cell, DIRECTIONS, MPI_DOUBLE, comm->rank_x-1,1,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
			// send right
			if(comm->rank_x != comm->nb_x-1){
				double * cell = lbm_mesh_get_cell(mesh, comm->width-2, i);
				MPI_Send((void*)cell, DIRECTIONS, MPI_DOUBLE, comm->rank_x+1,1,MPI_COMM_WORLD);
			}
		}
	}

}
