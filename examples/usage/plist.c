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

void test_plist_union(void)
{
	bc_plist_t *a, *b, *c;

	a = bc_plist_create(7, 7, 1, 5, 6, 2, 3, 9);
	b = bc_plist_create(5, 2, 1, 6, 9, 8);
	c = bc_plist_union(2, a, b);

	if (bc_rank == 0) {
		bc_plist_display(a);
		bc_plist_display(b);
		bc_plist_display(c);
	}

	bc_plist_destroy(a);
	bc_plist_destroy(b);
	bc_plist_destroy(c);
}

void test_plist_isect(void)
{
	bc_plist_t *a, *b, *c;

	a = bc_plist_create(7, 7, 1, 5, 6, 2, 3, 9);
	b = bc_plist_create(5, 2, 1, 6, 9, 8);
	c = bc_plist_isect(2, a, b);

	if (bc_rank == 0) {
		bc_plist_display(a);
		bc_plist_display(b);
		bc_plist_display(c);
	}

	bc_plist_destroy(a);
	bc_plist_destroy(b);
	bc_plist_destroy(c);
}

void test_plist_diff(void)
{
	bc_plist_t *a, *b, *c;

	a = bc_plist_create(7, 7, 1, 5, 6, 2, 3, 9);
	b = bc_plist_create(5, 2, 1, 6, 9, 8);
	c = bc_plist_diff(a, b);

	if (bc_rank == 0) {
		bc_plist_display(a);
		bc_plist_display(b);
		bc_plist_display(c);
	}

	bc_plist_destroy(a);
	bc_plist_destroy(b);
	bc_plist_destroy(c);
}

void test_plist_default(void)
{
	bc_plist_display(bc_plist_all);
	bc_plist_display(bc_plist_odd);
	bc_plist_display(bc_plist_even);
	bc_plist_display(bc_plist_pred);
	bc_plist_display(bc_plist_succ);
}

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);
	bc_init(BC_PLIST_ALL | BC_PLIST_SUCC); 

 	test_plist_union();
 	test_plist_isect();
 	test_plist_diff();
	test_plist_default(); 

	bc_final();
	MPI_Finalize();
	return 0;
}
