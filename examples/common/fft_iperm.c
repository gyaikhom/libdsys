/***************************************************************************
 * $Id: fft.c,v 1.1.1.1 2004/06/18 23:03:26 s0231576 Exp $
 *
 *  Saturday June 12 00:57:02 2004
 *  Copyright  2004  Gagarine Yaikhom
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
 * Example implementation of the Fast Fourier Transformation.
 * Input permutation phase, so that output data are ordered.
 *
 * USAGE: 
 *         mpirun c0-3 ./fft -- 32
 *
 * For a test run on 4 processors with 32 coefficients distributed evenly.
 *
 *
 * Reference:
 * Michael J. Quinn, "Parallel Computing - Theory and Practice.",
 * McGraw-Hill Inc., 1994
 */

#include <mpi.h>
#include <math.h>
#include <dsys.h>

#define XFIG_PREAMBLE "#FIG 3.2\nLandscape\nCenter\nInches\nLetter\n100.00\nSingle\n-2\n1200 2\n"
#define XFIG_LINE "2 1 0 1 %d 0 50 -1 -1 0.000 0 0 -1 0 0 4\n\t"	\
	"%d 100 %d 500 %d 1500 %d 2000\n"
#define XFIG_TEXT "4 0 0 50 -1 2 14 0.0000 4 150 4515 100 2500 " \
	"Fast Fourier Transformation: Coefficient Permutaion\\001\n" \
	"4 0 4 50 -1 0 14 0.0000 4 150 4515 100 2700 " \
	"%d coefficients with %d processes\\001\n" \

/*
 * The color values are XFig pen color values.
 * We are using only 13 colors.
 */
int colors[] = {0, 1, 2, 3, 4, 5, 8, 12, 15, 18, 20, 21, 24};

double log2(double x);
static unsigned int bit_reverse(unsigned int in, int nbits);
static int permute(int num_nodes, int ncoeff, double **coeff);

int main(int argc, char **argv)
{
	int i, j, ncoeff, ncoeff_node;
	double *coeff, *pcoeff;

	MPI_Init(&argc, &argv);
	bc_init(BC_ERR);

	if (argc != 2) {
		fprintf (stderr, "USAGE: mpirun c0-3 fft -- <number of coeff.>\n");
		exit (1);
	}

	/* Get number of coefficients */
	ncoeff = atoi(argv[1]);

	if ((bc_size > ncoeff) || (bc_size % 2)) {
		if (bc_rank == 0)
			fprintf(stderr, "ERROR: More nodes than coefficients, or odd number of nodes.\n");
		bc_final();
		MPI_Finalize();
		return 0;
	}
	
	/* Number of coefficients per node. */
	ncoeff_node = (unsigned int) (ncoeff/bc_size);

	/* Fill coefficients */
	coeff = (double *) malloc(sizeof(double) * ncoeff_node);
	pcoeff = (double *) malloc(sizeof(double) * ncoeff_node);
	for (i = 0; i < ncoeff_node; i++) {
		coeff[i] = (double)(i + bc_rank * ncoeff_node);
		pcoeff[i] = coeff[i];
	}

	/* Permute coefficients. */
	permute(bc_size, ncoeff, &pcoeff);

	/* Generate XFIG graphics file. */
	if (bc_rank == 0) {
		bc_chan_t *src;
		bc_plist_t *plist;
		FILE *xfigfile;

		xfigfile = fopen("fft_permute.fig", "w+");
		fprintf(xfigfile, XFIG_PREAMBLE);
		for (i = 0; i < ncoeff_node; i++)
			fprintf(xfigfile, XFIG_LINE, colors[bc_rank % 13],
					(int) coeff[i]*200,	(int) coeff[i]*200,
					(int) pcoeff[i]*200, (int) pcoeff[i]*200);

		/*
		 * Get and print coefficients from other nodes.
		 * We can also use BC_ROLE_COLLECT if we allocate enough
		 * space for coeff and pcoeff.
		 */
		for (i = 1; i < bc_size; i++) {
			plist = bc_plist_create(1, i);
			src = bc_src_create(plist, bc_double, BC_ROLE_PIPE);

			/* Get proginal and permuted coefficients from remote. */
			bc_get(src, coeff, ncoeff_node);
			bc_get(src, pcoeff, ncoeff_node);
			bc_chan_destroy(src);
			bc_plist_destroy(plist);

			for (j = 0; j < ncoeff_node; j++)
				fprintf(xfigfile, XFIG_LINE, colors[i % 13],
						(int) coeff[j]*200,	(int) coeff[j]*200,
						(int) pcoeff[j]*200, (int) pcoeff[j]*200);
		}
		fprintf(xfigfile, XFIG_TEXT, ncoeff, bc_size);
		fclose(xfigfile);
		printf("Output file 'fft_permute.fig' successfully generated...\n"
			   "Please use XFig to view.\n");
	}else {
		bc_chan_t *sink;
		bc_plist_t *plist;

		plist = bc_plist_create(1, 0);
		sink = bc_sink_create(plist, bc_double, ncoeff_node, BC_ROLE_PIPE);

		/* Send my original, and permuted coefficients. */
		bc_put(sink, coeff, ncoeff_node);
		bc_put(sink, pcoeff, ncoeff_node);

		bc_chan_destroy(sink);
		bc_plist_destroy(plist);
	}

	free(coeff); free(pcoeff);
	bc_final();
	MPI_Finalize();
	return 0;
}

/* Permutation phase of the FFT. */
int permute(int num_nodes, int ncoeff, double **coeff)
{
	unsigned int reverse_bits;
	int nbits, ncoeff_node, nrem_nodes, current, i, j, k;
	int *node, *index, temp;
	double index_coeff[2], *temp_coeff;
	bc_chan_t **src, **sink;
	bc_plist_t **plists;

	/* Number of remote nodes */
	nrem_nodes = num_nodes - 1;

	if (!(src = (bc_chan_t **) malloc(sizeof(bc_chan_t *) * nrem_nodes)))
		return -1;
	if (!(sink = (bc_chan_t **) malloc(sizeof(bc_chan_t *) * nrem_nodes))) {
		free(src);
		return -1;
	}
	if (!(plists = (bc_plist_t **) malloc(sizeof(bc_plist_t *) * num_nodes))) {
		free(src);
		free(sink);
		return -1;
	}

	/* Number of coefficients per node */
	ncoeff_node = (int) (ncoeff/num_nodes); 

	if (!(temp_coeff = (double *) malloc(sizeof(double) * ncoeff_node))) {
		free(sink);
		free(src);
		return -1;
	}

	/* Cache for storing nodes and index values */
	if (!(node = (int *) malloc(sizeof(int) * ncoeff_node))) {
		free(temp_coeff);
		free(sink);
		free(src);
		return -1;
	}
	if (!(index = (int *) malloc(sizeof(int) * ncoeff_node))) {
		free(node);
		free(temp_coeff);
		free(sink);
		free(src);
		return -1;
	}

	/* Number of bits for bit reversing */
	nbits = (int) log2((double) ncoeff);

	/*
	 * Create branching channels.
	 */
	plists[num_nodes] = bc_plist_create(1, bc_rank);
	temp = ncoeff_node << 1; /* Let us use double buffer. */
	for (i = j = 0; i < num_nodes; i++) {
		if (i != bc_rank) {
			plists[j] = bc_plist_create(1, i);
			sink[j] = bc_sink_create(plists[j], bc_double, temp, BC_ROLE_PIPE);
			src[j] = bc_src_create(plists[j], bc_double, BC_ROLE_PIPE);
			j++;
		}
	}

	/* Compute nodes and indices for permutation */
	temp = bc_rank * ncoeff_node;
	for (i = 0; i < ncoeff_node; i++) {
		current =  temp + i;
		reverse_bits = bit_reverse(current, nbits);
		node[i] = reverse_bits / ncoeff_node;
		index[i] = (int) (reverse_bits % ncoeff_node);
	}

	/* Put permuted values for remote */
	for (i = 0; i < ncoeff_node; i++) {
		if (node[i] != bc_rank) { 
			index_coeff[0] = (double) index[i]; /* Set the index */
			index_coeff[1] = *(*coeff + i);     /* Set the coefficient */
			k = ((node[i] > bc_rank) ? (node[i] - 1) : node[i]);

			/*
			 * We can use a custom data type created with bc_dtype_create(),
			 * which will make us send only 1 data unit instead of two doubles.
			 */
			bc_put(sink[k], index_coeff, 2);
		} else {
			temp_coeff[i] = *(*coeff + index[i]);
		}
	}

	/* Get permuted values from remote */
	for (i = 0; i < ncoeff_node; i++) {
		if (node[i] != bc_rank) {
			k = ((node[i] > bc_rank) ? (node[i] - 1) : node[i]);
			bc_get(src[k], index_coeff, 2);
			*(*coeff + (int) index_coeff[0]) = index_coeff[1];
		} else {
			*(*coeff + i) = temp_coeff[i];
		}
	}

	/* Destroy branching channels and process lists. */
	for (i = 0; i < nrem_nodes; i++) {
		bc_chan_destroy(src[i]);
		bc_chan_destroy(sink[i]);
		bc_plist_destroy(plists[i]);
	}
	bc_plist_destroy(plists[num_nodes]);

	free(temp_coeff); free(node); free(index);
	free(src); free(sink); free(plists);

	return 0;
}

/*
 * Reverse The 'nbits' Bits of 'num'
 */
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
