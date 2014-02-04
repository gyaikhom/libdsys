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
 * Example implementation of odd-even sorting (with memory copy).
 *
 * USAGE: 
 *         mpirun -c 4 ./odd_even -- 4096 10
 *
 * For a test run of 10 sortings on 4 processors with 4096 integers distributed 
 * evenly.
 *
 *
 * NOTE:
 * The code is adapted from the MPI implementation of Odd-Even Sort found in
 * Ananth Grama et.al, "Introduction to Parallel Programming", Second Edition,
 * Addison-Wesley, pages: 249-250
 */

#include <mpi.h>
#include <dsys.h>

static void compare_accum(int nlocal, int *elements, int *workspace, short keepsmall);
static int inc_order(const void *x, const void *y);

static int local_upper, recv_upper, *recv_base;
static size_t bytes;

int main(int argc, char **argv)
{
    int odd_rank, even_rank, n, nlocal, i, j, repeat;
    int *workspace, *elements, iter;
    double start_time;
	bc_plist_t *odd_pl = NULL, *even_pl = NULL;
    bc_chan_t *odd_src = NULL, *odd_sink = NULL;
    bc_chan_t *even_src = NULL, *even_sink = NULL;

    /* Initialize MPI and bc. */
    MPI_Init(&argc, &argv);
    bc_init(BC_ERR);

    repeat = atoi(argv[2]);

    /* Get the total number of elements to be sorted. */
    n = atoi(argv[1]);

	/* Get the start time. */
    start_time = bc_gettime_sec();

    nlocal = n / bc_size;

    /* Allocate working space. */
    if (!(elements  = (int *) calloc(nlocal, sizeof(int)))) {
		perror("Allocationg space");
		exit(1);
	}
    if (!(workspace = (int *) calloc(nlocal << 1, sizeof(int)))) {
		free(elements);
		perror("Allocating space");
		exit(1);
	}
    recv_base = workspace + nlocal;

    /* These values need not be calculated during the loop. */
    local_upper = nlocal - 1;
    recv_upper = (nlocal << 1) - 1;
    bytes = nlocal*sizeof(int);

	if (bc_rank % 2) {
		odd_rank = bc_rank - 1;
		even_rank = bc_rank + 1;
	} else {
		odd_rank = bc_rank + 1;
		even_rank = bc_rank - 1;
	}
    
    /* Create the branching channels for data sharing. */
	if (odd_rank > -1 && odd_rank < bc_size) {
		odd_pl = bc_plist_create(1, odd_rank);		
		odd_src = bc_src_create(odd_pl, bc_int, BC_ROLE_PIPE);
		odd_sink = bc_sink_create(odd_pl, bc_int, nlocal, BC_ROLE_PIPE);
	}
	if (even_rank > -1 && even_rank < bc_size) {
		even_pl = bc_plist_create(1, even_rank);
		even_src = bc_src_create(even_pl, bc_int, BC_ROLE_PIPE);
		even_sink = bc_sink_create(even_pl, bc_int, nlocal, BC_ROLE_PIPE);
	}

	iter = bc_size / 2;
    /* Number of sortings. */
    for (j = 0;  j < repeat; j++) {
	    /* Initialise random number generator seed. */
	    srandom(bc_rank+1+j);

	    /* Fill local elements with random integers. */
	    for (i = 0; i < nlocal; i++) {
	        elements[i] =  random() % 9000;
	    }

 	    printf ("[%d] Unsorted: ", bc_rank); 
 	    for (i = 0; i < nlocal; i++)
 	        printf ("%d ", elements[i]);
 	    printf ("\n"); 

	    /* Quicksort the local elements. */
	    qsort(elements, nlocal, sizeof(int), inc_order);

	    /* Main Odd-Even Sort loop. */
	    for (i = 0; i < iter; i++) {
			if (even_pl) {
				bc_put(even_sink, elements, nlocal);
				bc_get(even_src, recv_base, nlocal);
				compare_accum(nlocal, elements, workspace, bc_rank < even_rank);
			} 
			if (odd_pl) {
				bc_put(odd_sink, elements, nlocal);
				bc_get(odd_src, recv_base, nlocal);
				compare_accum(nlocal, elements, workspace, bc_rank < odd_rank);
			}
		}

        printf ("[%d] Sorted: ", bc_rank);
        for (i = 0; i < nlocal; i++)
            printf ("%d ", elements[i]);
        printf ("\n");
    }

    printf ("[%d] Total time: %f seconds\n", bc_rank,
		bc_gettime_sec() - start_time);

    /* Destroy the branching channels. */
	if (odd_pl) {
		bc_chan_destroy(odd_src);
		bc_chan_destroy(odd_sink);
		bc_plist_destroy(odd_pl);
	}

	if (even_pl) {
		bc_chan_destroy(even_src); 
		bc_chan_destroy(even_sink);
		bc_plist_destroy(even_pl);
	}

    /* Deallocate resources. */
    free(elements);
	free(workspace);

    /* Finalize MPI and bc layer. */
    bc_final();
    MPI_Finalize();

    return 0;
}

/* Compare and accumulate large or small elements in 'elements'. */
void
compare_accum(int nlocal, int *elements, int *workspace, short keepsmall)
{
    int i, j, k;

    memcpy(workspace, elements, bytes);

    if (keepsmall) {
        i = 0;
	    j = nlocal;
        for (k = 0; k < nlocal; k++) {
            if (workspace[i] < workspace[j])
		        elements[k] = workspace[i++];
            else 
		        elements[k] = workspace[j++];
        }
    } else {
        i = local_upper;
	    j = recv_upper;
        for (k = i; k >= 0; k--) {
            if (workspace[i] >= workspace[j])
		        elements[k] = workspace[i--];
            else 
		        elements[k] = workspace[j--];
        }
    }
}

int
inc_order(const void *x, const void *y)
{
    return (*(int *)x - *(int *)y);
}
