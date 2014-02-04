/***************************************************************************
 * $Id: reduce_oper.c,v 1.2 2004/11/06 10:59:00 project Exp $
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
 * branching channel 'sink'. The consumer reduces all the data
 * produced by the producers - SUMMATION and MULTIPLY.
 *
 * Usage: mpirun -c 6 reduce
 */
 
#include <mpi.h>
#include <dsys.h> 

#define NUM_DATA         5
#define NUM_PRODS        5
#define PRODUCERS        1, 2, 3, 4, 5
#define WITH_MEMORY_COPY 1

void producer(void);
void consumer(void);

void custom_reduce (void *buffer, int offset, int count) {
	int i, value = 0;
	for (i = 0; i < count; i++) {
		if (i % 2)
			value -= *((int *) (buffer + offset * count * i));
		else
			value += *((int *) (buffer + offset * count * i));
	}
	*(int *) buffer = value;
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    bc_init(BC_ERR); 

	if (bc_size < NUM_PRODS+1) {
		if (bc_rank == 0)
			printf("ERROR: %d processes are required.\n"
				   "\n\tUSAGE: mpirun -c %d reduce\n\n",
				   NUM_PRODS+1, NUM_PRODS+1);
		bc_final();
		MPI_Finalize();
		return -1;
	}

	bc_reduce_operator = custom_reduce;
    switch (bc_rank) {
    case 0:
        consumer();
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
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

    printf("[%d] Data:", bc_rank);
    srandom(bc_rank);
    for (i = 0; i < NUM_DATA; i++) {
        value[i] = random()%50;
        printf(" %2d", value[i]);
    }
    printf("\n");

	cons = bc_plist_create(1, 0);
    
#if WITH_MEMORY_COPY
	/* For Sum reduction. */
	sink = bc_sink_create(cons, bc_int, NUM_DATA, BC_ROLE_PIPE);
    bc_put(sink, value, NUM_DATA);
	bc_chan_destroy(sink);

	/* For multiplicative reduction. */
	sink = bc_sink_create(cons, bc_int, NUM_DATA, BC_ROLE_PIPE);
    bc_put(sink, value, NUM_DATA);
    bc_chan_destroy(sink);

	/* For minimum value reduction. */
	sink = bc_sink_create(cons, bc_int, NUM_DATA, BC_ROLE_PIPE);
    bc_put(sink, value, NUM_DATA);
    bc_chan_destroy(sink);

	/* For maximum value reduction. */
	sink = bc_sink_create(cons, bc_int, NUM_DATA, BC_ROLE_PIPE);
    bc_put(sink, value, NUM_DATA);
    bc_chan_destroy(sink);

	/* For minimum and maximum value reduction. */
	sink = bc_sink_create(cons, bc_int, NUM_DATA, BC_ROLE_PIPE);
    bc_put(sink, value, NUM_DATA);
    bc_chan_destroy(sink);

	/* For operator reduction. */
	sink = bc_sink_create(cons, bc_int, NUM_DATA, BC_ROLE_PIPE);
    bc_put(sink, value, NUM_DATA);
    bc_chan_destroy(sink);

#else

	/* For Sum reduction. */
	sink = bc_sink_create(cons, bc_int, NUM_DATA, BC_ROLE_PIPEN);
    for (i = 0; i < NUM_DATA; i++) {
        bc_var(sink, int) = value[i];
		bc_commit(sink);
	}
	bc_chan_destroy(sink);

	/* For multiplicative reduction. */
	sink = bc_sink_create(cons, bc_int, NUM_DATA, BC_ROLE_PIPEN);
    for (i = 0; i < NUM_DATA; i++) {
        bc_var(sink, int) = value[i];
		bc_commit(sink);
	}
    bc_chan_destroy(sink);

	/* For minimum value reduction. */
	sink = bc_sink_create(cons, bc_int, NUM_DATA, BC_ROLE_PIPEN);
    for (i = 0; i < NUM_DATA; i++) {
        bc_var(sink, int) = value[i];
		bc_commit(sink);
	}
    bc_chan_destroy(sink);

	/* For maximum value reduction. */
	sink = bc_sink_create(cons, bc_int, NUM_DATA, BC_ROLE_PIPEN);
    for (i = 0; i < NUM_DATA; i++) {
        bc_var(sink, int) = value[i];
		bc_commit(sink);
	}
    bc_chan_destroy(sink);

	/* For minimum and maximum value reduction. */
	sink = bc_sink_create(cons, bc_int, NUM_DATA, BC_ROLE_PIPEN);
    for (i = 0; i < NUM_DATA; i++) {
        bc_var(sink, int) = value[i];
		bc_commit(sink);
	}
    bc_chan_destroy(sink);

	/* For operator reduction. */
	sink = bc_sink_create(cons, bc_int, NUM_DATA, BC_ROLE_PIPEN);
    for (i = 0; i < NUM_DATA; i++) {
        bc_var(sink, int) = value[i];
		bc_commit(sink);
	}
    bc_chan_destroy(sink);

#endif

    bc_plist_destroy(cons);
}

void consumer(void) {
    bc_chan_t *src;
    bc_plist_t *prod;
    int value[NUM_DATA << 1], i;

	prod = bc_plist_create(NUM_PRODS, PRODUCERS);

#if WITH_MEMORY_COPY    
	src = bc_src_create(prod, bc_int, BC_ROLE_REDUCE_SUM);
	bc_get(src, value, NUM_DATA);
    printf("\nSum reduction:\n");
    for (i = 0; i < NUM_DATA; i++)
	    printf ("[%d] Value[%d] = %d\n", bc_rank, i, value[i]);
	bc_chan_destroy(src);

	src = bc_src_create(prod, bc_int, BC_ROLE_REDUCE_MUL);
	bc_get(src, value, NUM_DATA);
    printf("\nMultiplicative reduction:\n");
    for (i = 0; i < NUM_DATA; i++)
	    printf ("[%d] Value[%d] = %d\n", bc_rank, i, value[i]);
	bc_chan_destroy(src);
    
    src = bc_src_create(prod, bc_int, BC_ROLE_REDUCE_MIN);
	bc_get(src, value, NUM_DATA);
    printf("\nMinimum reduction:\n");
    for (i = 0; i < NUM_DATA; i++)
	    printf ("[%d] Value[%d] = %d\n", bc_rank, i, value[i]);
	bc_chan_destroy(src);
    
    src = bc_src_create(prod, bc_int, BC_ROLE_REDUCE_MAX);
	bc_get(src, value, NUM_DATA);
    printf("\nMaximum reduction:\n");
    for (i = 0; i < NUM_DATA; i++)
	    printf ("[%d] Value[%d] = %d\n", bc_rank, i, value[i]);
	bc_chan_destroy(src);
    
    src = bc_src_create(prod, bc_int, BC_ROLE_REDUCE_MINMAX);
	bc_get(src, value, NUM_DATA);
    printf("\nMinimum and maximum reduction:\n");
    for (i = 0; i < NUM_DATA; i++)
	    printf ("[%d] MIN[%d] = %2d MAX[%d] = %2d \n", bc_rank, i,
				value[i], i, value[i+NUM_DATA]);
	bc_chan_destroy(src);

    src = bc_src_create(prod, bc_int, BC_ROLE_REDUCE_OPT);
	bc_get(src, value, NUM_DATA);
    printf("\nOperator reduction:\n");
    for (i = 0; i < NUM_DATA; i++)
	    printf ("[%d] Value[%d] = %2d\n", bc_rank, i, value[i]);
	bc_chan_destroy(src);

#else

	src = bc_src_create(prod, bc_int, BC_ROLE_REDUCE_SUM);
    printf("\nSum reduction:\n");
    for (i = 0; i < NUM_DATA; i++) {
		bc_get(src, value, 1);
	    printf ("[%d] Value[%d] = %d\n", bc_rank, i, value[0]);
	}
	bc_chan_destroy(src);

	src = bc_src_create(prod, bc_int, BC_ROLE_REDUCE_MUL);
    printf("\nMultiplicative reduction:\n");
    for (i = 0; i < NUM_DATA; i++) {
		bc_get(src, value, 1);
	    printf ("[%d] Value[%d] = %d\n", bc_rank, i, value[0]);
	}
	bc_chan_destroy(src);
    
    src = bc_src_create(prod, bc_int, BC_ROLE_REDUCE_MIN);
    printf("\nMinimum reduction:\n");
    for (i = 0; i < NUM_DATA; i++) {
		bc_get(src, value, 1);
	    printf ("[%d] Value[%d] = %d\n", bc_rank, i, value[0]);
	}
	bc_chan_destroy(src);
    
    src = bc_src_create(prod, bc_int, BC_ROLE_REDUCE_MAX);
    printf("\nMaximum reduction:\n");
    for (i = 0; i < NUM_DATA; i++) {
		bc_get(src, value, 1);
	    printf ("[%d] Value[%d] = %d\n", bc_rank, i, value[0]);
	}
	bc_chan_destroy(src);
    
    src = bc_src_create(prod, bc_int, BC_ROLE_REDUCE_MINMAX);
    printf("\nMinimum and maximum reduction:\n");
    for (i = 0; i < NUM_DATA; i++) {
		bc_get(src, value, 1);
	    printf ("[%d] MIN[%d] = %2d MAX[%d] = %2d \n", bc_rank, i, value[0], i, value[1]);
	}
	bc_chan_destroy(src);

    src = bc_src_create(prod, bc_int, BC_ROLE_REDUCE_OPT);
    printf("\nOperator reduction:\n");
    for (i = 0; i < NUM_DATA; i++) {
		bc_get(src, value, 1);
	    printf ("[%d] Value[%d] = %2d\n", bc_rank, i, value[0]);
	}
	bc_chan_destroy(src);

#endif

    bc_plist_destroy(prod);
}
