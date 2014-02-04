/***************************************************************************
 * $Id: replicate.c,v 1.1 2004/10/25 09:34:50 project Exp $
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

#define NUM_CONSUMERS    5
#define CONSUMERS        1, 2, 3, 4, 5
#define WITH_MEMORY_COPY 1

void producer(void);
void consumer(void);

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    bc_init(BC_ERR); 

	if (bc_size < NUM_CONSUMERS+1) {
		if (bc_rank == 0)
			printf("ERROR: %d processes are required.\n"
				   "\n\tUSAGE: mpirun -c %d replicate\n\n",
				   NUM_CONSUMERS+1, NUM_CONSUMERS+1);
		bc_final();
		MPI_Finalize();
		return -1;
	}

    switch (bc_rank) {
    case 0:
		producer();
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        consumer();
        break;
    default:
        printf ("[%d] Nothing to do.\n", bc_rank);
    }
    
    bc_final();
    MPI_Finalize();
    return 0;
}

void producer(void)
{
    bc_chan_t *sink;
    bc_plist_t *cons;
    int value = 0, i;

	cons = bc_plist_create(NUM_CONSUMERS, CONSUMERS);

#if WITH_MEMORY_COPY
  	sink = bc_sink_create(cons, bc_int, 10, BC_ROLE_REPLICATE);
#else
  	sink = bc_sink_create(cons, bc_int, 5, BC_ROLE_REPLICATEN);
#endif

	for (i = 0; i < 10; i++, value++) {
#if WITH_MEMORY_COPY
  	    bc_put(sink, &value, 1);
#else
		bc_var(sink, int) = value;
		bc_commit(sink);
#endif
        printf ("[%d] Putting: %d\n", bc_rank, value);
    }
        
 	bc_chan_destroy(sink);
    bc_plist_destroy(cons);
}

void consumer(void)
{
    bc_chan_t *src;
    bc_plist_t *prod;
    int value, i;

	prod = bc_plist_create(1, 0);

#if WITH_MEMORY_COPY
  	src = bc_src_create(prod, bc_int, BC_ROLE_PIPE);
#else
  	src = bc_src_create(prod, bc_int, BC_ROLE_PIPE);
#endif

	for (i = 0; i < 10; i++) {
	    bc_get(src, &value, 1);
	    printf ("[%d] Value = %d\n", bc_rank, value);
	}

   	bc_chan_destroy(src);
    bc_plist_destroy(prod);
}
