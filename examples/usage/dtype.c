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

/**
 * This program illustrate
 * 1. Complex data types manipulation.
 * 2. Usage of bc_vptr().
 *
 * USAGE: mpirun -c 2 dtype
 */
 
#include <mpi.h>
#include <dsys.h> 
#include <string.h>

/* Custom data structure. */
struct foo {
	int number;
	char name[20];
};

int main(int argc, char *argv[])
{
	bc_chan_t *bc;
	bc_plist_t *pl;
	bc_dtype_t *ntype;
	struct foo temp;

	MPI_Init(&argc, &argv);
	bc_init(BC_ERR); 

	if (bc_size != 2) {
		if (bc_rank == 0)
			printf("ERROR: 2 processes are required.\n"
				   "\n\tUSAGE: mpirun -c 2 dtype\n\n");
		bc_final();
		MPI_Finalize();
		return -1;
	}
    
	ntype = bc_dtype_create(sizeof(struct foo));

	if (bc_rank == 0) {
		pl = bc_plist_create(1, 1);
		bc = bc_sink_create(pl, ntype, 2, BC_ROLE_PIPEN);

		bc_vptr(bc, struct foo)->number = bc_rank;
		strcpy(bc_vptr(bc, struct foo)->name, "Hello World!");

		bc_commit(bc);

		bc_plist_destroy(pl);
		bc_chan_destroy(bc);
	} else {
		pl = bc_plist_create(1, 0);
		bc = bc_src_create(pl, ntype, BC_ROLE_PIPE);

		temp.number = bc_rank;
		strcpy(temp.name, "Where are you?");
		printf("Process %d: \"%s\"\n", temp.number, temp.name);

		bc_get(bc, &temp, 1);
		printf("Process %d: \"%s\"\n", temp.number, temp.name);

		bc_plist_destroy(pl);
		bc_chan_destroy(bc);
	}

	bc_dtype_destroy(ntype);

	bc_final();
	MPI_Finalize();
	return 0;
}
