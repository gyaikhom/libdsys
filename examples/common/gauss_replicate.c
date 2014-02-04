/***************************************************************************
 * $Id$
 *
 *  Saturday June 12 00:57:02 2004
 *  Copyright  2004  Gagarine Yaikhom
 *  gyaikhom@gmail.com
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
#define WITH_MEM_COPY 0 /* Set to 1 for buffering with memory copy. */
#define DIMENSION     5 /* Choose the dimension: number of variables. */

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

/* Use to index the last column: y vector. */
int	y = DIMENSION + 1;

void gaussian_seq(void)
{
	int i, j, k;

	/* Do for all the rows. */
	for (i = 0; i < dim; i++) {

		/* Divide current row. */
		for (j = i+1; j < dim; j++)
			A[i][j] = A[i][j]/A[i][i];
		A[i][y] = A[i][dim]/A[i][i];
		A[i][i] = 1;

		/* Eliminate preceding rows. */
		for (k = i+1; k < dim; k++) {
			for (j = i+1; j < dim; j++)
				A[k][j] = A[k][j] - A[k][i]*A[i][j];
			A[k][dim] = A[k][dim] - A[k][i] * A[i][y];
			A[k][i] = 0;
		}
	}
}

void gaussian_par_nmcpy(void)
{
	int i, j, last;
	bc_plist_t *sink_pl = NULL, *src_pl = NULL;
	bc_chan_t *sink = NULL, *src = NULL;
	bc_dtype_t *ntype;

	last = bc_size - 1;

	/* Create a custom datatype. Used for avoiding
	 * memory copy during buffering.
	 */
	ntype = bc_dtype_create(sizeof(float)*len);

	/* Create the branching channel. */
	if (bc_rank != last) {
		int i, j;
		
		sink_pl = bc_plist_create_empty(bc_size - bc_rank - 1);
		for (i = bc_rank + 1, j = 0; i < bc_size; i++, j++)
			bc_plist_set(sink_pl, j, i);

		sink = bc_sink_create(sink_pl, ntype, 2, BC_ROLE_REPLICATEN);
	}

	/* Eliminate all the preceding rows. */
	for (i = 0; i < bc_rank; i++) {
		src_pl = bc_plist_create(1, i);
		src = bc_src_create(src_pl, ntype, BC_ROLE_PIPE);

		/* Receive preceding row for elimination. */
		bc_get(src, &A[i][0], 1);

		for (j = i+1; j < dim; j++)
			A[bc_rank][j] = A[bc_rank][j] - A[bc_rank][i] * A[i][j];
		A[bc_rank][dim] = A[bc_rank][dim] - A[bc_rank][i] * A[i][y];
		A[bc_rank][i] = 0;

		bc_chan_destroy(src);
		bc_plist_destroy(src_pl);
	}

	/* Preceding rows have been eliminated, divide my row. */
	for (j = bc_rank+1; j < dim; j++)
		A[bc_rank][j] = A[bc_rank][j]/A[bc_rank][bc_rank];
	A[bc_rank][y] = A[bc_rank][dim]/A[bc_rank][bc_rank];
	A[bc_rank][bc_rank] = 1;

	/* I have completed, send my row for following processes to eliminate. */
	if (bc_rank != last) {
		for (i = 0; i < len; i++)
			*(bc_vptr(sink, float) + i) = A[bc_rank][i];
		bc_commit(sink);
	}

	/* Destroy producer branching channel and process list. */
	if (bc_rank != last) {
		bc_chan_destroy(sink);
		bc_plist_destroy(sink_pl);
	}

	/* Destroy the custom data type. */
	bc_dtype_destroy(ntype);
}

/* Parallel Version: Buffering with memory copy. */
void gaussian_par_mcpy(void)
{
	int i, j, last;
	bc_plist_t *sink_pl = NULL, *src_pl = NULL;
	bc_chan_t *sink = NULL, *src = NULL;

	/* Create the process list. */
	last = bc_size - 1;

	/* Create the branching channel. */
	if (bc_rank != last) {
		int i, j;
		
		sink_pl = bc_plist_create_empty(bc_size - bc_rank - 1);
		for (i = bc_rank + 1, j = 0; i < bc_size; i++, j++)
			bc_plist_set(sink_pl, j, i);

		sink = bc_sink_create(sink_pl, bc_float, (y+1) << 1, BC_ROLE_REPLICATE);
	}

	/* Eliminate all the preceding rows. */
	for (i = 0; i < bc_rank; i++) {
		src_pl = bc_plist_create(1, i);
		src = bc_src_create(src_pl, bc_float, BC_ROLE_PIPE);

		/* Receive preceding row for elimination. */
		bc_get(src, &A[i][0], y+1);

		for (j = i+1; j < dim; j++)
			A[bc_rank][j] = A[bc_rank][j] - A[bc_rank][i] * A[i][j];
		A[bc_rank][dim] = A[bc_rank][dim] - A[bc_rank][i] * A[i][y];
		A[bc_rank][i] = 0;

		bc_chan_destroy(src);
		bc_plist_destroy(src_pl);
	}

	/* Preceding rows have been eliminated, divide my row. */
	for (j = bc_rank+1; j < dim; j++)
		A[bc_rank][j] = A[bc_rank][j]/A[bc_rank][bc_rank];
	A[bc_rank][y] = A[bc_rank][dim]/A[bc_rank][bc_rank];
	A[bc_rank][bc_rank] = 1;

	/* I have completed, send my row for following processes to eliminate. */
	if (bc_rank != last)
		bc_put(sink, &A[bc_rank][0], y+1);

	/* Destroy producer branching channel and process list. */
	if (bc_rank != last) {
		bc_chan_destroy(sink);
		bc_plist_destroy(sink_pl);
	}
}

int main(int argc, char *argv[])
{
	int i;
	double start;

  	MPI_Init(&argc, &argv);
  	bc_init(BC_ERR);
	start = bc_gettime_usec();

#if SEQUENTIAL
	if (bc_rank == 0) {
		int j;

		gaussian_seq();
		
		printf("Ux = y\n\nMatrix U:\n");
		for (i = 0; i < dim; i++) {
			for (j = 0; j < dim; j++)
				printf("%8.5f ", A[i][j]);
			printf("\n");
		}
		printf("\nVector y:\n");
		for (i = 0; i < dim; i++)
			printf("[%d] %8.5f\n", bc_rank, A[i][y]);
	}
#else
	if (bc_size != dim) {
		if (bc_rank == 0)
			printf("ERROR: %d processes are required.\n"
				   "\n\tUSAGE: mpirun -c %d gauss_replicate\n\n",
				   dim, dim);
		bc_final();
		MPI_Finalize();
		return -1;
	}

#if WITH_MEM_COPY
	gaussian_par_mcpy();
#else
	gaussian_par_nmcpy();
#endif

 	for (i = 0; i < dim; i++)
 		printf("U[%d, %d] = %8.5f\n", bc_rank, i, A[bc_rank][i]);
	printf("y[%d] = %8.5f\n", bc_rank, A[bc_rank][y]);

#endif

#if SEQUENTIAL
	if (bc_rank == 0)
		printf("\nTime: %f\n", bc_gettime_usec() - start);
#else
	printf("Time[%d]: %f usecs\n\n", bc_rank, bc_gettime_usec() - start);
#endif
  	bc_final();
  	MPI_Finalize();

    return 0;
}
