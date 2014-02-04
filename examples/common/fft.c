/***************************************************************************
 * $Id$
 *
 *  Saturday June 12 00:57:02 2004
 *  Copyright  2004  Gagarine Yaikhom
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

/*
 * Example implementation of the Fast Fourier Transformation.
 * Type: one task per process, communications in all iterations.
 *
 * USAGE: 
 *         mpirun -c NPROC fft
 *
 *
 * Reference:
 * Michael J. Quinn, "Parallel Computing - Theory and Practice.",
 * McGraw-Hill Inc., 1994
 */

#include <math.h>
#include <stdio.h>
#include <mpi.h>
#include <dsys.h>

#define TEST1 0
#define TEST2 0
#define TEST3 0
#define TEST4 1

#define PI2 6.283185307
#define bit_comp(num, bit) ((1 << (bit)) ^ (num))

struct complex_s {
	double r;
	double i;
};
typedef struct complex_s complex_t;

static unsigned int bit_reverse(unsigned int num, int nbits);
extern double log2(double);
static int complex_mul(complex_t *r, complex_t *a, complex_t *b);
static int complex_pow(complex_t *r, complex_t *a, int p);
static int omega(complex_t *d, int i);
static int fft(int nnodes, int ncoeff, complex_t *coeff);
static int permute(int num_nodes, complex_t coeff[]);

int main(int argc, char *argv[])
{
	complex_t coeff;
	int nproc;

#if TEST1
	complex_t page201_1[2] = {{2.0, 0.0},
							  {3.0, 0.0}};
	nproc = 2;
#endif
#if TEST2
 	complex_t page201_2[4] = {{1.0, 0.0},
							  {2.0, 0.0},
							  {4.0, 0.0},
							  {3.0, 0.0}};
	nproc = 4;
#endif
#if TEST3
 	complex_t page204[8] = {{-1.0, 0.0},
							{ 5.0, 0.0},
							{-4.0, 0.0},
							{ 2.0, 0.0},
							{ 0.0, 0.0},
							{ 0.0, 0.0},
							{ 0.0, 0.0},
							{ 0.0, 0.0}};
	nproc = 8;
#endif
#if TEST4
	complex_t page205[8] = {{2.0, 0.0},
							{3.0, 0.0},
							{2.0, 0.0},
							{1.0, 0.0},
							{0.0, 0.0},
							{0.0, 0.0},
							{0.0, 0.0},
							{0.0, 0.0}};
	nproc = 8;
#endif

	MPI_Init(&argc, &argv);
	bc_init(BC_ERR);

	if (bc_size < nproc) {
		if (bc_rank == 0)
			printf("ERROR: %d processes are required.\n"
				   "\n\tUSAGE: mpirun -c %d fft\n\n",
				   nproc, nproc);
		bc_final();
		MPI_Finalize();
		return -1;
	}


#if TEST1
	/* Test 1: Page: 201_1.*/
	if (bc_rank < 2) {
		if (bc_rank == 0)
			printf("Test 1: page 201, example 1\n");
		permute(2, page201_1);
		coeff.r = page201_1[bc_rank].r;
		coeff.i = page201_1[bc_rank].i;
		fft(2, 2, &coeff);
		printf("[%2d] %5.2f %5.2fi\n", bc_rank, coeff.r, coeff.i);
	}
#endif
#if TEST2
	/* Test 2: Page: 201_2.*/
	if (bc_rank < 4) {
		if (bc_rank == 0)
			printf("Test 2: page 201, example 2\n");
		permute(4, page201_2);
		coeff.r = page201_2[bc_rank].r;
		coeff.i = page201_2[bc_rank].i; 
		fft(4, 4, &coeff);
		printf("[%2d] %5.2f %5.2fi\n", bc_rank, coeff.r, coeff.i);
	}
#endif
#if TEST3
	/* Test 1: Page: 204.*/
	if (bc_rank < 8) {
		if (bc_rank == 0)
			printf("Test 3: page 204\n");
		permute(8, page204);
		coeff.r = page204[bc_rank].r;
		coeff.i = page204[bc_rank].i; 
		fft(8, 8, &coeff);
		printf("[%2d] %5.2f %5.2fi\n", bc_rank, coeff.r, coeff.i);
	}
#endif
#if TEST4
	/* Test 1: Page: 205.*/
	if (bc_rank < 8) {
		if (bc_rank == 0)
			printf("Test 4: page 205\n");
		permute(8, page205);
		coeff.r = page205[bc_rank].r;
		coeff.i = page205[bc_rank].i; 
		fft(8, 8, &coeff);
		printf("[%2d] %5.2f %5.2fi\n", bc_rank, coeff.r, coeff.i);
	}
#endif

	bc_final();
	MPI_Finalize();
	return 0;
}

/*
 * Calculate the Fourier Transform with FFT.
 * This example is for the one-task-per-process variant.
 * Hence, (nnodes != ncoeff) is ERROR.
 */
int fft(int nnodes, int ncoeff, complex_t *coeff)
{
	unsigned int i, j, step;
	int iter, nbit, cpnode, nbc, idx, pbit, lsbit, *partner, sign;
	complex_t recv, om, temp, prod;
	bc_plist_t **plists;
	bc_chan_t **src, **sink;
	bc_dtype_t *ntype;
	
	cpnode = ncoeff/nnodes;            /* Coefficients per node. */
	iter = (int) log2((double)ncoeff); /* Iterations per node. */
	pbit = (int) log2((double)nnodes); /* Bits in process rank. */
	nbc = iter*cpnode;				   /* No. of branching channels. */
	idx = bc_rank*cpnode;			   /* My coefficient index. */
	lsbit = iter - pbit;			   /* No. of bits in ncoeff - pbits. */

	/* Create communication structure. */
	ntype = bc_dtype_create(sizeof(complex_t));
	partner = (int *) malloc(sizeof(int)*nbc);
	plists = (bc_plist_t **) malloc(sizeof(bc_plist_t *)*nbc);
	src = (bc_chan_t **) malloc(sizeof(bc_chan_t *)*nbc);
	sink = (bc_chan_t **) malloc(sizeof(bc_chan_t *)*nbc);

	for (i = iter, j = 0; i; i--) {
		nbit = iter - i;
			partner[j] = bit_comp(idx, nbit) >> lsbit;
			plists[j] = bc_plist_create(1, partner[j]);
			src[j] = bc_src_create(plists[j], ntype, BC_ROLE_PIPE);
			sink[j] = bc_sink_create(plists[j], ntype, 2, BC_ROLE_PIPE);
			j++;
	}

	/* Execute communication structure. */
	step = 0;
	for (i = 1, j = 0; i <= iter; i++) {
			/*
			 * Multiply with omega:
			 * If I am communicating with a node (partner) with
			 * lesser rank than me. 
			 */
			if (partner[j] < bc_rank) {
				/*
				 * Get 2**i primitive root of unity.
				 * NOTE: This is very inefficient, but serves
				 * the purpose of demonstrating how branching
				 * channels work. Real applications should make
				 * this more efficient.
				 */
				omega(&om, 1 << i);
				complex_pow(&temp, &om, step);
				complex_mul(&prod, coeff, &temp);
				*coeff = prod;
				sign = -1;
				step += i;
			} else
				sign = 1;
			
			bc_put(sink[j], coeff, 1);
			bc_get(src[j], &recv, 1);

			if (sign > 0) {
				coeff->r = recv.r + coeff->r; 
				coeff->i = recv.i + coeff->i;
			} else {
				coeff->r = recv.r - coeff->r; 
				coeff->i = recv.i - coeff->i;
			}
			j++;
	}

	/* Destroy communication structure. */
	for (i = 0; i < nbc; i++) {
		bc_chan_destroy(src[i]);
		bc_chan_destroy(sink[i]);
		bc_plist_destroy(plists[i]);
	}
 	bc_dtype_destroy(ntype);
	free(src); free(sink); free(plists);

	return 0;
}

/* Reverse The 'nbits' Bits of 'num'. */
unsigned int bit_reverse(unsigned int num, int nbits) 
{ 
	int i;
	unsigned int reverse = 0;

	for (i = 0; i < nbits; i++) { 
		reverse = (reverse << 1) | (num & 1);
		num = (num >> 1);
	}

	return reverse; 
}

/* Input Permutation phase of the FFT. */
int permute(int nnodes, complex_t coeff[])
{
	int nbits, node;

	/* Number of bits for bit reversing */
	nbits = (int) log2((double) nnodes);

	/* Compute nodes and indices for permutation */
	node = bit_reverse((unsigned int)bc_rank, nbits);
	if (node != bc_rank)
		coeff[bc_rank] = coeff[node];

	return 0;
}

int complex_mul(complex_t *r, complex_t *a, complex_t *b)
{
	r->r = a->r*b->r - a->i*b->i;
	r->i = a->r*b->i + a->i*b->r;
	return 0;
}

int complex_pow(complex_t *r, complex_t *a, int p)
{
	double c;
	int i;

	if (p == 0) {
		r->r = 1.0;
		r->i = 0.0;
		return 0;
	}

	r->r = a->r;
	r->i = a->i;
	for (i = 1; i < p; i++) {
		c = r->r*a->r - r->i*a->i;
		r->i = r->r*a->i + r->i*a->r;
		r->r = c;
	}
	return 0;
}

/* Returns the ith primitive root of unity. */
int omega(complex_t *d, int i)
{
	double theta = PI2/i;
	d->r = cos(theta);
	d->i = sin(theta);
	return 0;
}
