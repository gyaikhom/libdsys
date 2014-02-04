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
 *
 * Adapted from the hypercube Algorithm in Michael J. Quinn,
 * 'Parallel Computing: Theory and Practice', Second Edition, McGraw Hill,
 * 1994, pp. 194.
 *
 * Usage: mpirun -c 4 matrix_mul
 */

#include <mpi.h>
#include <dsys.h>
#include <stdlib.h>
#include <sys/file.h>

#define NUM_SLAVES  3
#define SLAVE_PLIST 1, 2, 3

/**
 * This file is generated using 'matrix_gen'. Please generate a test set
 * (pair of matrices) by using this program. And, then, we can compile the
 * program as usual.
 */
#include "matrices.dat"

static void display_matrix(int nr, int nc, int *m);
static int transpose(int r, int c, int *in);
static int multiply_matrices(int nra, int nca, int *a, int nrb, int ncb, int *b);
static int multiply_blocks(int *results, int nrblk, int c, int *rows, int ncblk,
						   int r, int *cols, int n);
static int multiply_row_column(int *row, int *col, int c);

void
display_matrix(int nr, int nc, int *m)
{
    long    i, j, k = 0;
    
    for (i = 0; i < nr; i++) {
        for (j = 0; j < nc; j++)
            printf("%3d ", m[k++]);
        printf("\n");
    }
}

int
transpose(int r, int c, int *in)
{
    int     *temp;
    int     i, j;
    
    temp = (int *) calloc(r*c, sizeof(int));
    
    for (i = 0; i < r; i++)
        for (j = 0; j < c; j++) {
            *(temp + j*r + i) = *(in + i*c + j);
        }
        
    memcpy(in, temp, r*c*sizeof(int));
    free(temp);
        
    return 0;
}

int
multiply_matrices(int nra, int nca, int *a, int nrb, int ncb, int *b)
{
    int nrblk, ncblk, rb_size, cb_size, i;
    int src_rank, sink_rank, last_rank;
    int *rows, *cols, *result;
    bc_plist_t *src_pl, *sink_pl;
    bc_chan_t *src, *sink;

    nrblk = nra/bc_size;    /* Number of rows per row block. */
    ncblk = ncb/bc_size;    /* Number of columns per column block. */
    rb_size = nrblk*nca;	  /* Size of the row block of matrix A. */
    cb_size = ncblk*nrb;	  /* Size of the column block of matrix B */
    
    /* Allocate working memory. */
    rows = (int *) calloc(rb_size, sizeof(int));
    cols = (int *) calloc(cb_size, sizeof(int));
    
    /** RANK = 0 is the master, which will reduce all the partial
     * rows from all the other slave processes to display the final
     * result. Allocate enough space for the final result. For the
     * slave nodes, allocate the result enough to fit a row block.
     */
    if (bc_rank == 0)
        result = (int *) calloc(nra*ncb, sizeof(int));
    else
        result = (int *) calloc(nrblk*ncb, sizeof(int));
    
    /* Get the initial row block of A and column block of B. For
     * getting the column block, first transpose and then copy the
     * appropriate rows. This will only work with contiguous memory
     * and with row-major representation of matrix B.
     */
    memcpy(rows, a + rb_size*bc_rank, rb_size*sizeof(int));
    transpose(nrb, ncb, b);  
    memcpy(cols, b + cb_size*bc_rank, cb_size*sizeof(int));

    /* Create a Ring topology. */
    last_rank = bc_size - 1;
    src_rank  = (bc_rank == last_rank) ? 0 : bc_rank + 1;
    sink_rank = (bc_rank == 0) ? last_rank : bc_rank - 1;
    
    /* Create branching channels for column block sharing. */
    src_pl  = bc_plist_create(1, src_rank);
    sink_pl = bc_plist_create(1, sink_rank);
    src     = bc_src_create(src_pl, bc_int, BC_ROLE_PIPE);
    sink    = bc_sink_create(sink_pl, bc_int, cb_size, BC_ROLE_PIPE);

    /* Calculate the row block which belong to me. */
    multiply_blocks(result, nrblk, nca, rows, ncblk, ncb, cols, 0);
    for (i = 1; i < bc_size; i++) {
        bc_put(sink, cols, cb_size); /* Put column block for successor. */
        bc_get(src, cols, cb_size);  /* Get column block from predecessor. */
        multiply_blocks(result, nrblk, nca, rows, ncblk, ncb, cols, i);
    }
    
    /* We do not need these branching channels, so free resources. */
	bc_chan_destroy(src); bc_chan_destroy(sink);
    bc_plist_destroy(src_pl); bc_plist_destroy(sink_pl); 
    
    /** If I am the master, request and collect partial row blocks from
     * all the slaves. If I am slave, send my row block to the master.
     */
    rb_size = nrblk*ncb; /* Update the row block size. */
    if (bc_rank == 0) {
        src_pl = bc_plist_create(NUM_SLAVES, SLAVE_PLIST);
        src = bc_src_create(src_pl, bc_int, BC_ROLE_COLLECT);
        bc_get(src, result + rb_size, rb_size);
        bc_chan_destroy(src);
        bc_plist_destroy(src_pl);                
        
        printf("\nResult:\n");
        display_matrix(nra, ncb, result);
    } else {
        sink_pl = bc_plist_create(1, 0);
        sink = bc_sink_create(sink_pl, bc_int, rb_size, BC_ROLE_PIPE);
        bc_put(sink, result, rb_size);
        bc_chan_destroy(sink);
        bc_plist_destroy(sink_pl);
    }

    /* Free working memory. */   
    free(rows); free(cols); free(result);
    return 0;
}

int
multiply_blocks(int *result, int nrblk, int ca, int *rows, int ncblk, int cb, int *cols, int n)
{
    /* 
     * k = (n + bc_rank) % bc_size;
     * temp = result + k*ncblk; 
     * for (i = 0; i < nrblk; i++) {
     *     for (j = 0; j < ncblk; j++)
     *         *(temp + j) = mult_row_column(rows + i*ca, cols + j*ca, ca);
     *     temp += cb;
     * }
     */
    int     i, j;
    int     *c, *temp;
    
    result += ((n + bc_rank) % bc_size)*ncblk;
    temp = result;
    
    for (i = 0; i < nrblk; i++) {
        c = cols;
        for (j = 0; j < ncblk; j++) {
            *(temp + j) = multiply_row_column(rows, c, ca);
            c += ca;
        }
        rows += ca;
        temp += cb;
    }
    return 0;
}


int
multiply_row_column(int *row, int *col, int c)
{
    int     result = 0;
    int     i;
    
    for (i = 0; i < c; i++)
        result += *(row + i) * *(col + i);
    
    return result;
}

int
main (int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    bc_init(BC_ERR);
       
    if (bc_rank == 0) {
        printf("Matric A:\n");
        display_matrix(NROWS_A, NCOLS_A, (int *)matrix_a);        
        printf("\nMatric B:\n");
        display_matrix(NROWS_B, NCOLS_B, (int *)matrix_b);
    }
    
    multiply_matrices(NROWS_A, NCOLS_A, matrix_a, NROWS_B, NCOLS_B, matrix_b);
    
    bc_final();
    MPI_Finalize();
    return 0;
}
