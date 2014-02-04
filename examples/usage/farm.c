/***************************************************************************
 * $Id: pipeline.c,v 1.1 2004/10/25 09:34:50 project Exp $
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
 
#include <mpi.h>
#include <dsys.h>

#define NUM_WORKERS 5
#define WORKERS     1, 2, 3, 4, 5

void farmer(bc_plist_t *pl);
void worker(bc_plist_t *pl);

int main(int argc, char *argv[])
{
    bc_plist_t *pl;

    MPI_Init(&argc, &argv);
    bc_init(BC_ERR); 

	if (bc_size < NUM_WORKERS+1) {
		if (bc_rank == 0)
			printf("ERROR: %d processes are required.\n"
				   "\n\tUSAGE: mpirun -c 6 farm\n\n", NUM_WORKERS+1);
		bc_final();
		MPI_Finalize();
		return -1;
	}

	switch(bc_rank) {
	case 0:
		pl = bc_plist_create(NUM_WORKERS, WORKERS);
		farmer(pl);
 		bc_plist_destroy(pl);
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		pl = bc_plist_create(1, 0);
		worker(pl);
 		bc_plist_destroy(pl);
		break;
	default:
		printf("Nothing to do.\n");
	}

 	bc_final();
	MPI_Finalize();
	return 0;
}

void farmer(bc_plist_t *pl)
{
	bc_chan_t *bc;
	int i;

	bc = bc_sink_create(pl, bc_int, 100, BC_ROLE_FARMN);

	for (i = 0; i < 1000; i++) {
		bc_var(bc, int) = i;
		bc_commit(bc);
	}

 	bc_chan_destroy(bc);
	printf("[%d] Farmer Done\n", bc_rank);
}

void worker(bc_plist_t *pl)
{
	bc_chan_t *bc;
	int i, value;

	bc = bc_src_create(pl, bc_int, BC_ROLE_PIPE);

	for (i = 0; i < 200; i++) {
		bc_get(bc, &value, 1);
		printf("[%d] %d\n", bc_rank, value);
	}

 	bc_chan_destroy(bc);
	printf("[%d] Worker Done\n", bc_rank);
}
