/***************************************************************************
 * $Id$
 *
 *  Copyright 2004 Gagarine Yaikhom
 *  Copyright 2004 University of Edinburgh
 *  s0231576@sms.ed.ac.uk
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * Gaussian Elimination: n-degree with n processors.
 *
 * Reference:
 * Ananth Grama, Ansul Gupta, George Karypis, Vipin Kumar,
 * "Introduction to Parallel COmputing"
 * Second Edition
 * Addison Wesley, 2003
 */

#include <mpi.h>
#include <dsys.h>

#define SEQUENTIAL    0 /* Set to 1 for sequential implementation. */
#define WITH_MEM_COPY 1 /* Set to 1 for buffering with memory copy. */
#define DIMENSION     5 /* Choose the dimension: number of variables. */
#define NUM_STAGES    5
#define PROCESSES     0, 1, 2, 3, 4

/* Some test cases. */
#if (DIMENSION == 5)
int dim = DIMENSION;
int len = DIMENSION + 2;
float A[DIMENSION][DIMENSION+2] = 
	{{3.0, 2.0, 1.0, 5.0, 1.0, 2.0, 0.0},
	 {1.0, 1.0, 1.0, 7.0, 2.0, 3.0, 0.0},
	 {3.0, 3.0, 1.0, 2.0, 1.0, 1.0, 0.0},
	 {1.0, 1.0, 2.0, 3.0, 3.0, 2.0, 0.0},
	 {1.0, 2.0, 3.0, 1.0, 3.0, 1.0, 0.0}};
#endif
#if (DIMENSION == 4)
int dim = DIMENSION;
int len = DIMENSION + 2;
float A[DIMENSION][DIMENSION+2] = 
	{{ 2.0, 10.0, -5.0,  1.0,  6.0, 0.0},
	 { 1.0,  3.0,  4.0,  2.0,  0.0, 0.0},
	 {-3.0,  6.0, 12.0, -4.0, 16.0, 0.0},
	 { 0.0,  4.0,  3.0,  3.0, 20.0, 0.0}};
#endif
#if (DIMENSION == 3)
int dim = DIMENSION;
int len = DIMENSION + 2;
float A[DIMENSION][DIMENSION+2] = 
	{{ 3.0, -20.0,  1.0, 12.0, 0.0},
	 { 1.0,   6.0, -4.0,  8.0, 0.0},
	 {-1.0,   3.0,  5.0,  3.0, 0.0}};
#endif

/* Used in indexing the last column: y vector. */
int	y = DIMENSION + 1;

/* Parallel Version: Buffering with memory copy. */
void gaussian_mcpy_stage (void **var, bc_chan_t *src, bc_chan_t *sink)
{
	int i, j, last;

	/* Create the process list. */
	last = bc_size - 1;

	/* Eliminate all the preceding rows. */
	for (i = 0; i < bc_rank; i++) {
		/* Receive preceding row for elimination. */
		bc_get(src, &A[i][0], y+1);

		for (j = i+1; j < dim; j++)
			A[bc_rank][j] = A[bc_rank][j] - A[bc_rank][i] * A[i][j];
		A[bc_rank][dim] = A[bc_rank][dim] - A[bc_rank][i] * A[i][y];
		A[bc_rank][i] = 0;

		/* Pass on the row received from the predecing process. */
		if (bc_rank != last)
			bc_put(sink, &A[i][0], y+1);
	}

	/* Preceding rows have been eliminated, divide my row. */
	for (j = bc_rank+1; j < dim; j++)
		A[bc_rank][j] = A[bc_rank][j]/A[bc_rank][bc_rank];
	A[bc_rank][y] = A[bc_rank][dim]/A[bc_rank][bc_rank];
	A[bc_rank][bc_rank] = 1;

	/* I have completed, send my row for following processes to eliminate. */
	if (bc_rank != last)
		bc_put(sink, &A[bc_rank][0], y+1);
}

int main(int argc, char *argv[])
{
	int i;
	iskel_pipe_t *pipe; 	/* The pipeline topology instance. */

	/*
	 * Data map which gives the input and output branching channel
	 * types for each stage in the pipeline.
	 */
	iskel_pipe_dmap_t dmap[]
		= {{bc_float, bc_float},
		   {bc_float, bc_float},
		   {bc_float, bc_float},
		   {bc_float, bc_float},
		   {bc_float, bc_float}};

	/* Pipeline stage functions. */
	iskel_pipe_fptr_t func[] = {gaussian_mcpy_stage,
								gaussian_mcpy_stage,
								gaussian_mcpy_stage,
								gaussian_mcpy_stage,
								gaussian_mcpy_stage};

	/* Processes participating in the pipeline. */
	bc_plist_t *plist;

  	MPI_Init(&argc, &argv);
  	bc_init(BC_ERR);

	if (bc_size != dim) {
		if (bc_rank == 0)
			printf("ERROR: %d processes are required.\n"
				   "\n\tUSAGE: mpirun -c %d gauss_pipeline\n\n",
				   dim, dim);
		bc_final();
		MPI_Finalize();
		return -1;
	}

	/* Create the process lists for the pipeline. */
 	plist = bc_plist_create(NUM_STAGES, PROCESSES);

	/* Create pipeline topology. */
	pipe = iskel_pipe_create(plist, func, dmap, 1);

	/* Execute topology. */
 	iskel_pipe_exec(pipe, NULL, NULL);

	/* Destroy the pipeline topology. */
 	iskel_pipe_destroy(pipe);

	/* Destroy the process list. */
	bc_plist_destroy(plist);

 	for (i = 0; i < dim; i++)
 		printf("U[%d, %d] = %8.5f\n", bc_rank, i, A[bc_rank][i]);
	printf("y[%d] = %8.5f\n", bc_rank, A[bc_rank][y]);

  	bc_final();
  	MPI_Finalize();

    return 0;
}
