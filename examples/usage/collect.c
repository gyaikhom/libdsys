/***************************************************************************
 * $Id: collect.c,v 1.1 2004/10/29 15:05:05 project Exp $
 *
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
#include <stdlib.h>

#define NUM_DATA    5
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
    int value[NUM_DATA], i;

    printf("[%d] ", bc_rank);
	srandom(bc_rank);
    for (i = 0; i < NUM_DATA; i++) {
        value[i] = random() % 100;
        printf(" %3d", value[i]);
    }
    printf("\n");

	cons = bc_plist_create(1, 0);    
    sink = bc_sink_create(cons, bc_int, NUM_DATA, BC_ROLE_PIPE);

	/* Data for column major. */
    bc_put(sink, value, NUM_DATA);

	/* Data for row major. */
    bc_put(sink, value, NUM_DATA);

    bc_chan_destroy(sink);
    bc_plist_destroy(cons);
}

void consumer(void)
{
    bc_chan_t *src;
    bc_plist_t *prod;
    int row[NUM_DATA*NUM_PRODS], col[NUM_DATA*NUM_PRODS];
	int i, j, k;

	prod = bc_plist_create(NUM_PRODS, PRODUCERS);
    src = bc_src_create(prod, bc_int, BC_ROLE_COLLECT);

	/* Column major. */
	for (i = 0; i < NUM_DATA; i++)
		bc_get(src, col+i*NUM_PRODS, 1);

	/* Row major. */
	bc_get(src, row, NUM_DATA);

    bc_chan_destroy(src);
    bc_plist_destroy(prod);

    printf("\n[%d] Column Major:\n", bc_rank);
    for (k = 0, i = 0; i < NUM_DATA; i++) {
		for (j = 0; j < NUM_PRODS; j++)
			printf ("%4d", col[k++]);
		printf("\n");
	}

    printf("\n[%d] Row Major:\n", bc_rank);
    for (k = 0, i = 0; i < NUM_PRODS; i++) {
		for (j = 0; j < NUM_DATA; j++)
			printf ("%4d", row[k++]);
		printf("\n");
	}
}
