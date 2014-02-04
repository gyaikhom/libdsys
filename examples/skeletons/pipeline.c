/***************************************************************************
 * $Id$
 *
 *  Copyright  2004  Gagarine Yaikhom
 *  Copyright  2004  University of Edinburgh
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

/**
 * For comparison with memory copy version of the pipeline reported 
 * in the paper:
 *
 * "Shared Message Buffering without Intermediate Memory Copy"
 *
 * Submitted to HLPP 2005.
 *
 * Usage:
 *    mpirun -c 4 pipeline
 *
 * NOTE: Tested succesfully with LAM-MPI 7.0.4.
 */

#include <mpi.h>
#include <dsys.h>

#define NUM_STAGES       4
#define PROCESSES        0, 1, 2, 3
#define WITH_MEMORY_COPY 0

/**
 * Stage function prototypes:
 *
 * The arguments are:
 * (1) Input or output variable.
 * (2) Input branching channel.
 * (3) Output branching channel.
 */
void stage_first(void **ivar, bc_chan_t *ibc, bc_chan_t *obc);
void stage_inter(void **ivar, bc_chan_t *ibc, bc_chan_t *obc);
void stage_last(void **ovar, bc_chan_t *ibc, bc_chan_t *obc);

int main(int argc, char *argv[])
{
	int i;
	iskel_pipe_t *pipe; 	/* The pipeline topology instance. */

	/*
	 * Data map which gives the input and output branching channel
	 * types for each stage in the pipeline.
	 */
	iskel_pipe_dmap_t dmap[] = {{bc_int, bc_int},
								{bc_int, bc_int},
								{bc_int, bc_int},
								{bc_int, bc_int}};

	/* Pipeline stage functions. */
	iskel_pipe_fptr_t func[] = {stage_first,
								stage_inter,
								stage_inter,
								stage_last};

	/* Processes participating in the pipeline. */
	bc_plist_t *plist;

	/* First and second data sets. */
	int in1[6] = {5, 4, 3, 2, 1, 0};
	int in2[11] = {11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0};

	/* First and second output variables. */
	int out1[6];
	int out2[11];

	/* Initialise libbc using MPI. */
	MPI_Init(&argc, &argv);
	bc_init(BC_ERR);
	
	if (bc_size != NUM_STAGES) {
		if (bc_rank == 0)
			printf("ERROR: %d processes are required.\n"
				   "\n\tUSAGE: mpirun -c %d pipeline\n\n",
				   NUM_STAGES, NUM_STAGES);
		bc_final();
		MPI_Finalize();
		return -1;
	}

	/* Create the process lists for the pipeline. */
 	plist = bc_plist_create(NUM_STAGES, PROCESSES);

	/* Create pipeline topology instance. */
#if WITH_MEMORY_COPY
	pipe = iskel_pipe_create(plist, func, dmap, 1);
#else
	pipe = iskel_pipe_create(plist, func, dmap, 0);
#endif

	/* Execute topology on the first data set. */
  	iskel_pipe_exec(pipe, in1, out1);

	/* Execute topology on the second data set. */
 	iskel_pipe_exec(pipe, in2, out2);

	/* Destroy the pipeline topology. */
 	iskel_pipe_destroy(pipe);

	/* Destroy the process list. */
	bc_plist_destroy(plist);

	/* Print the results. */
	if (bc_rank == 0) {
		printf("Input:\n");
		for (i = 0; i < 5; i++)
			printf("%d ", in1[i]);
		printf("\n--------------------------\n");
		printf("Input:\n");
		for (i = 0; i < 10; i++)
			printf("%d ", in2[i]);
		printf("\n--------------------------\n");
	} else if (bc_rank == 3) {
		printf("Output:\n");
		for (i = 0; i < 5; i++)
			printf("%d ", out1[i]);
		printf("\n--------------------------\n");
		printf("Output:\n");
		for (i = 0; i < 10; i++)
			printf("%d ", out2[i]);
		printf("\n--------------------------\n");
	}

	/* Finalise libbc. */
	bc_final();
	MPI_Finalize();

	return 0;
}

#if WITH_MEMORY_COPY

/* First stage. */
void stage_first(void **ivar, bc_chan_t *ibc, bc_chan_t *obc)
{
	int i, j = 0;

	while(1) {
		i = *(int *) (ivar + j);

		if (i != 0) {
			i += bc_rank;
			bc_put(obc, &i, 1);
		} else {
			bc_put(obc, &i, 1);
			break;
		}
		j++;
	}
}

/* Intermediate stage. */
void stage_inter(void **ivar, bc_chan_t *ibc, bc_chan_t *obc)
{
	int i;

	while(1) {
		bc_get(ibc, &i, 1);

		if (i != 0) {
			i +=bc_rank;
			bc_put(obc, &i, 1);
		} else {
			bc_put(obc, &i, 1);
			break;
		}
	}
}

/* Last stage. */
void stage_last(void **ovar, bc_chan_t *ibc, bc_chan_t *obc)
{
	int j = 0;

	while(1) {
		bc_get(ibc, (int *)ovar + j, 1);
		if (*((int *)ovar + j) != 0) {
			*((int *)ovar + j) += bc_rank;
		} else break;
		j++;
	}
}

#else

/* First stage. */
void stage_first(void **ivar, bc_chan_t *ibc, bc_chan_t *obc)
{
	int j = 0;

	while(1) {
		/*
		 * We use local variable abstraction of the output
		 * branching channel to do our computation.
		 */
		bc_var(obc, int) = *(int *) (ivar + j);

		if (bc_var(obc, int) != 0) {
			bc_var(obc, int) += bc_rank;
			bc_commit(obc);
		} else {
			bc_commit(obc);
			break;
		}
		j++;
	}
}

/* Intermediate stage. */
void stage_inter(void **ivar, bc_chan_t *ibc, bc_chan_t *obc)
{
	while(1) {
		/*
		 * We are using the local abstraction of the output
		 * branching channel for receiving data.
		 */
		bc_get(ibc, bc_vptr(obc, int), 1);

		if (bc_var(obc, int) != 0) {
			bc_var(obc, int) +=bc_rank;
			bc_commit(obc);
		} else {
			bc_commit(obc);
			break;
		}
	}
}

/* Last stage. */
void stage_last(void **ovar, bc_chan_t *ibc, bc_chan_t *obc)
{
	int j = 0;

	while(1) {
		bc_get(ibc, (int *)ovar + j, 1);
		if (*((int *)ovar + j) != 0) {
			*((int *)ovar + j) += bc_rank;
		} else break;
		j++;
	}
}

#endif
