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

/* Description:
 * Data exchange between two processes. To display dynamic branching
 * channel creation, we have divided the exchange into two phases.
 *
 * Usage: mpirun -c 2 exchange
 */
 
#include <mpi.h>
#include <dsys.h>

#define WITH_MEMORY_COPY 0
#define PRINT_DATA       1
#define PRINT_TIME       1
#define NUM_DATA         100

void exchange(int *value, bc_plist_t *pl);

int main(int argc, char *argv[])
{
    bc_plist_t *pl;
    int value[NUM_DATA], i;
#if PRINT_TIME
	double start;
#endif

    MPI_Init(&argc, &argv);
    bc_init(BC_ERR); 

#if PRINT_TIME
	start = bc_gettime_sec();
#endif

	if (bc_size != 2) {
		if (bc_rank == 0)
			printf("ERROR: 2 processes are required.\n"
				   "\n\tUSAGE: mpirun -c 2 exchange\n\n");
		bc_final();
		MPI_Finalize();
		return -1;
	}

    if (bc_rank == 0) {
        pl = bc_plist_create(1, 1);
        for (i = 0; i < NUM_DATA; i++)
            value[i] = (bc_rank+1)*100 + i;
		exchange(value, pl);
		bc_plist_destroy(pl);
	} else {
        pl = bc_plist_create(1, 0);
        for (i = 0; i < NUM_DATA; i++)
            value[i] = (bc_rank+1)*100 + i;
        exchange(value, pl);
		bc_plist_destroy(pl);
     }

#if PRINT_TIME
	printf("[%d] Time: %f\n", bc_rank, bc_gettime_sec() - start);
#endif

    bc_final();
    MPI_Finalize();
    return 0;
}

void exchange(int *value, bc_plist_t *pl)
{
    bc_chan_t *sink, *src;
    int i;

#if WITH_MEMORY_COPY
	sink = bc_sink_create(pl, bc_int, 2, BC_ROLE_PIPE);
	src = bc_src_create(pl, bc_int, BC_ROLE_PIPE);
#else
 	sink = bc_sink_create(pl, bc_int, 2, BC_ROLE_PIPEN);
 	src = bc_src_create(pl, bc_int, BC_ROLE_PIPEN);
#endif

  	for (i = 0; i < NUM_DATA; i++) {
#if WITH_MEMORY_COPY
	    bc_put(sink, &(value[i]), 1);
#else
		bc_var(sink, int) = value[i];
	    bc_commit(sink);
#endif
 		bc_get(src, &(value[i]), 1);

#if PRINT_DATA
		printf ("[%d] Value = %d\n", bc_rank, value[i]);
#endif
	}

 	bc_chan_destroy(src);
   	bc_chan_destroy(sink);
}
