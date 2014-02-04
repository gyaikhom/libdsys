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

#include <mpi.h>
#include <dsys.h>

#define WITH_MEMORY_COPY 0 /* Set to 1 for memory copy buffering. */

void farmer(void **in, void **out, bc_chan_t *src, bc_chan_t *sink);
void worker(void **in, void **out, bc_chan_t *src, bc_chan_t *sink);

int main(int argc, char *argv[])
{
	int i, j;
	float in[4][4];				   /* Input data. */
	int out[4][4];				   /* Output data */
	bc_plist_t *pl;			   /* Farm process collection. */
	iskel_farm_t *farm;			   /* Farm topology. */
	iskel_farm_dmap_t dmap[2] = {  /* Data map for branching channels. */
		{bc_int, bc_float},	   /* Farmer (input, output). */
		{bc_float, bc_int}};   /* Worker (input, output). */

	MPI_Init(&argc, &argv);
	bc_init(BC_ERR);

	/*
	 * Create the process collection for farming.
	 * NOTE: The firs node is the farmer, rest are worker.
	 */
	pl = bc_plist_create(5, 0, 1, 2, 3, 4);

	/* Create the farm topology. */
#if WITH_MEMORY_COPY
  	farm = iskel_farm_create(pl, farmer, worker, dmap, 1);
#else
  	farm = iskel_farm_create(pl, farmer, worker, dmap, 0);
#endif

	in[0][0] = 1.34;
	in[0][1] = 1.0;
	in[0][2] = 1.11;
	in[0][3] = 4.3;
	in[1][0] = 2.5;
	in[1][1] = 7.9;
	in[1][2] = 8.2;
	in[1][3] = 6.3;
	in[2][0] = 4.4;
	in[2][1] = 9.9;
	in[2][2] = 0.5;
	in[2][3] = 0.15;
	in[3][0] = 6.0;
	in[3][1] = 4.0;
	in[3][2] = 3.0;
	in[3][3] = 9.8;

	/* Execute the farm. */
  	iskel_farm_exec(farm, &in[0][0], &out[0][0]);

	if (bc_rank == 0) {
		for (i = 0; i < 4; i++) {
			for (j = 0; j < 4; j++)
				printf("%d ", out[i][j]);
			printf("\n"); 
		}
	}

   	iskel_farm_destroy(farm);
  	bc_plist_destroy(pl);

	/* Finalise libbc. */
	bc_final();
	MPI_Finalize();
	return 0;
}

#if WITH_MEMORY_COPY
void farmer(void **in, void **out, bc_chan_t *src, bc_chan_t *sink)
{
	int i;

	for (i = 0; i < 4; i++) {
		bc_put(sink, (float **)in + i*4, 1);
		bc_get(src, (int **)out + i*4, 1);
	}
}
void worker(void **in, void **out, bc_chan_t *src, bc_chan_t *sink)
{
	int i, data_out;
	float data_in;

	for (i = 0; i < 4; i++) {
		bc_get(src, &data_in, 1);
		data_out = (data_in > 1.12345 && data_in < 5.12345) ? 1 : 0;
		bc_put(sink, &data_out, 1);
	}
}
#else
void farmer(void **in, void **out, bc_chan_t *src, bc_chan_t *sink)
{
	int i;

	for (i = 0; i < 4; i++) {
		bc_put(sink, (float **)in + i*4, 1);
		bc_get(src, (int **)out + i*4, 1);
	}
}
void worker(void **in, void **out, bc_chan_t *src, bc_chan_t *sink)
{
	int i;
	float data;

	for (i = 0; i < 4; i++) {
		bc_get(src, &data, 1);
		bc_var(sink, int) = (data > 1.12345 && data < 5.12345) ? 1 : 0;
		bc_commit(sink);
	}
}
#endif
