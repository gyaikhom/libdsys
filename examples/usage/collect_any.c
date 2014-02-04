/***************************************************************************
 *  Copyright  2004  Gagarine Yaikhom
 *  Copyright  2004  University of Edinburgh
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

/* Description:
 * The producers produces some data, and puts them into the
 * branching channel 'sink'. The consumer collects all the data
 * produced by the producers.
 *
 * Usage: mpirun -c 6 collect
 */
 
#include <mpi.h>
#include <dsys.h> 
#include <unistd.h> 

#define MAX_DATA    10
#define NUM_PRODS   6
#define PRODUCERS   1, 2, 3, 4, 5, 6

void producer(void);
void consumer(void);

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    bc_init(BC_ERR); 

	if (bc_size < NUM_PRODS+1) {
		if (bc_rank == 0)
			printf("ERROR: %d processes are required.\n"
				   "\n\tUSAGE: mpirun -c %d collect\n\n",
				   NUM_PRODS+1, NUM_PRODS+1);
		bc_final();
		MPI_Finalize();
		return -1;
	}

    switch (bc_rank) {
    case 0:
        consumer();
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        producer();
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
    int value, i;

	cons = bc_plist_create(1, 0);    
    sink = bc_sink_create(cons, bc_int, MAX_DATA, BC_ROLE_PIPE);

	srandom(bc_rank);
    for (i = 0; i < MAX_DATA; i++) {
		value = 10*bc_rank + i;
		usleep(random() % 99999);
		bc_put(sink, &value, 1);
    }

    bc_chan_destroy(sink);
    bc_plist_destroy(cons);
}

void consumer(void)
{
    bc_chan_t *src;
    bc_plist_t *prod;
    int received[MAX_DATA*NUM_PRODS];
	int i, total;

	prod = bc_plist_create(NUM_PRODS, PRODUCERS);
    src = bc_src_create(prod, bc_int, BC_ROLE_COLLECT_ANY);

	total = MAX_DATA*NUM_PRODS;
	for (i = 0; i < total; i++) {
		bc_get(src, &received[i], 1);
		printf ("%d ", received[i]);
	}

    bc_chan_destroy(src);
    bc_plist_destroy(prod);
}
