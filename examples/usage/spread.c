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
 * The producer produces some data, and puts them into the
 * branching channel 'sink'. The consumers gets those items from
 * the branching channel 'src'. Note the manner in which branching
 * channels are created: producer uses a 'SPREAD' pattern, while
 * the consumer uses a 'PIPELINE' pattern. The data produced by the
 * producer is spread over the consumers.
 *
 * Usage: mpirun -c 6 spread
 */
 
#include <mpi.h>
#include <dsys.h> 

#define NUM_CONSUMERS 5
#define CONSUMERS     1, 2, 3, 4, 5

void producer(void);
void consumer(void);

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    bc_init(BC_ERR); 

	if (bc_size < NUM_CONSUMERS+1) {
		if (bc_rank == 0)
			printf("ERROR: %d processes are required.\n"
				   "\n\tUSAGE: mpirun -c %d spread\n\n",
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
    int value[10];
    int i;

	cons = bc_plist_create(NUM_CONSUMERS, CONSUMERS);
	sink = bc_sink_create(cons, bc_int, 10, BC_ROLE_SPREAD);

    for (i = 0; i < 10; i++)
        value[i] = 100+i;
	bc_put(sink, value, 2);
        
	bc_chan_destroy(sink);
    bc_plist_destroy(cons);
}

void consumer(void)
{
    bc_chan_t *src;
    bc_plist_t *prod;
    int value[2];

	prod = bc_plist_create(1, 0);
	src = bc_src_create(prod, bc_int, BC_ROLE_PIPE);
    
 	bc_get(src, value, 2);
	bc_chan_destroy(src);
    bc_plist_destroy(prod);

	printf ("[%d] Value1 = %d, value2 = %d\n", bc_rank, value[0], value[1]);
}
