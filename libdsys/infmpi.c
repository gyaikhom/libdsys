/***************************************************************************
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

#include "base.h"
#include "infmpi.h"
#include "mem.h"
#include <mpi.h>

int __bc_mpi_info_create(void) {
    register unsigned int i, j, count;
    int namelen = 0;

    if (!(bc_base_layer->mpi = (bc_mpi_t *) malloc(sizeof (bc_mpi_t)))) {
        return -1;
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &(bc_base_layer->mpi->rank));
    MPI_Comm_size(MPI_COMM_WORLD, &(bc_base_layer->mpi->size));

    if (!(bc_base_layer->mpi->hnames =
            bc_malloc(char *, bc_base_layer->mpi->size))) {
        free(bc_base_layer->mpi);
        return -1;
    }

    for (i = 0; i < bc_base_layer->mpi->size; i++) {
        if (!(bc_base_layer->mpi->hnames[i] =
                bc_malloc(char, MPI_MAX_PROCESSOR_NAME + 1))) {
            for (j = 0; j < i; j++)
                bc_free(bc_base_layer->mpi->hnames[j]);
            bc_free(bc_base_layer->mpi->hnames);
            free(bc_base_layer->mpi);
            return -1;
        }
    }

    MPI_Get_processor_name(bc_base_layer->mpi->hnames[bc_base_layer->mpi->rank],
            &namelen);

    for (i = 0; i < bc_base_layer->mpi->size; i++) {
        if (i == bc_base_layer->mpi->rank)
            count = namelen + 1;
        else
            count = MPI_MAX_PROCESSOR_NAME + 1;
        MPI_Bcast(bc_base_layer->mpi->hnames[i], count, MPI_CHAR, i,
                MPI_COMM_WORLD);
    }
    return 0;
}

int __bc_mpi_info_destroy(void) {
    register unsigned int i;

    for (i = 0; i < bc_base_layer->mpi->size; i++)
        bc_free(bc_base_layer->mpi->hnames[i]);

    bc_free(bc_base_layer->mpi->hnames);
    free(bc_base_layer->mpi);

    return 0;
}
