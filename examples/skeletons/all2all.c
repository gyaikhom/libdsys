/***************************************************************************
 * $Id$
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

#include <mpi.h>
#include <dsys.h>

int main(int argc, char *argv[])
{
	iskel_all2all_t *all2all;
	bc_plist_t *pl;
	int i;
	float in, out[4];

	MPI_Init(&argc, &argv);
	bc_init(BC_ERR|BC_PLIST_ALL);

 	pl = bc_plist_create(3, 0, 1, 2);
  	all2all = iskel_all2all_create(pl, bc_float);

	in = bc_rank*1.83;
  	iskel_all2all_exec(all2all, &in, out, float);

	if (bc_plist_iselem(pl, bc_rank)) {
		printf("[%d] ", bc_rank);
		for (i = 0; i < bc_plist_nelem(pl); i++)
			printf("%f ", out[i]);
		printf("\n"); 
	}

	in = bc_rank*3.98;
 	iskel_all2all_exec(all2all, &in, out, float);

	if (bc_plist_iselem(pl, bc_rank)) {
		printf("[%d] ", bc_rank);
		for (i = 0; i < bc_plist_nelem(pl); i++)
			printf("%f ", out[i]);
		printf("\n"); 
	}

   	iskel_all2all_destroy(all2all);
  	bc_plist_destroy(pl);

	/* Finalise libbc. */
	bc_final();
	MPI_Finalize();
	return 0;
}

