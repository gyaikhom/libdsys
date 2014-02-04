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

/*
 * Please refer to the document pipeline_farm.ps for
 * a description.
 *
 * USAGE: mpirun -c 9 pipeline_farm
 */

#include <mpi.h>
#include <dsys.h>

/* Executed by process 0. */
void stage_0(void *in);

/* Executed by process 1. */
void stage_1(void);

/* Executed by process 4, 5, 6, 7 and 8. */
void stage_2(void);

/* Executed by process 2. */
void stage_3(void);

/* Executed by process 2. */
void stage_4(void *out);

int main(int argc, char *argv[])
{
	int *in, *out, i;

	/* Initialise libbc using MPI. */
	MPI_Init(&argc, &argv);
	bc_init(BC_ERR);

	if (bc_size != 9) {
		if (bc_rank == 0)
			printf("ERROR: 9 processes are required.\n"
				   "\n\tUSAGE: mpirun -c 9 pipeline_farm\n\n");
		bc_final();
		MPI_Finalize();
		return -1;
	}

	/* SPMD, per process stage function. */
	switch(bc_rank) {
	case 0:
		in = (int *) malloc(sizeof(int)*100);
		printf("[%d] Input:\n", bc_rank);
		for (i = 0; i < 100; i++) {
			in[i] = i;
  			printf("%d ", in[i]);
		}
		printf("\n");
		stage_0(in);
		free(in);
		break;
	case 1:
		stage_1();
		break;
	case 2:
		stage_3();
		break;
	case 3:
		out = (int *) malloc(sizeof(int)*2);
		stage_4(out);
 		printf("[%d] Sum: %d\n", bc_rank, out[0]);
		free(out);
		break;
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
		stage_2();
		break;
	default :
		break;
	}
	bc_final();
	MPI_Finalize();
	return 0;
}

void stage_0(void *in)
{
	bc_plist_t *pl;
	bc_chan_t *bc;
	int i = 0;

	pl = bc_plist_create(1, 1);
	bc = bc_sink_create(pl, bc_int, 10, BC_ROLE_PIPEN);

	for (i= 0; i < 100; i++) {
		bc_var(bc, int) = *((int *)in + i);
		bc_commit(bc);
	}
	bc_chan_destroy(bc);
	bc_plist_destroy(pl);
}

void stage_1(void)
{
	bc_plist_t *ipl, *opl;
	bc_chan_t *ibc, *obc;
	int i = 0;

	ipl = bc_plist_create(1, 0);
	opl = bc_plist_create(5, 4, 5, 6, 7, 8);
	ibc = bc_src_create(ipl, bc_int, BC_ROLE_PIPE);
	obc = bc_sink_create(opl, bc_int, 10, BC_ROLE_FARMN);

	for (i= 0; i < 100; i++) {
		bc_get(ibc, bc_vptr(obc, int), 1);
		bc_commit(obc);
	}

	bc_chan_destroy(ibc);
	bc_chan_destroy(obc);
	bc_plist_destroy(ipl);
	bc_plist_destroy(opl);
}

void stage_2(void)
{
	bc_plist_t *ipl, *opl;
	bc_chan_t *ibc, *obc;
	int i = 0;

	ipl = bc_plist_create(1, 1);
	opl = bc_plist_create(1, 2);
	ibc = bc_src_create(ipl, bc_int, BC_ROLE_PIPE);
	obc = bc_sink_create(opl, bc_int, 5, BC_ROLE_PIPEN);

	for (i= 0; i < 20; i++) {
		bc_get(ibc, bc_vptr(obc, int), 1);
		bc_commit(obc);
	}

	bc_chan_destroy(ibc);
	bc_chan_destroy(obc);
	bc_plist_destroy(ipl);
	bc_plist_destroy(opl);
}

void stage_3(void)
{
	bc_plist_t *ipl, *opl;
	bc_chan_t *ibc, *obc;
	int i = 0;

	ipl = bc_plist_create(5, 4, 5, 6, 7, 8);
	opl = bc_plist_create(1, 3);
	ibc = bc_src_create(ipl, bc_int, BC_ROLE_REDUCE_SUM);
	obc = bc_sink_create(opl, bc_int, 5, BC_ROLE_PIPEN);

	for (i= 0; i < 20; i++) {
		bc_get(ibc, bc_vptr(obc, int), 1);
		bc_commit(obc);
	}

	bc_chan_destroy(ibc);
	bc_chan_destroy(obc);
	bc_plist_destroy(ipl);
	bc_plist_destroy(opl);
}

void stage_4(void *out)
{
	bc_plist_t *ipl;
	bc_chan_t *ibc;
	int i = 0;

	ipl = bc_plist_create(1, 2);
	ibc = bc_src_create(ipl, bc_int, BC_ROLE_PIPE);

	*((int *) out + 0) = 0;
	for (i= 0; i < 20; i++) {
		bc_get(ibc, (int *)out + 1, 1);
		*((int *) out + 0) += *((int *) out + 1);
	}

	bc_chan_destroy(ibc);
	bc_plist_destroy(ipl);
}
