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

#include "base.h"
#include "mem.h"

/*
 * The internal data structures that forms the abstraction layer provided by
 * the Data System Framework Model.
 */
bc_base_t *bc_base_layer;

/*
 * Layer options for interface functionality.
 */
unsigned long __bc_internal_flags = BC_NULL;

/*
 * Initialises the Data System Framework Layer.
 */
int bc_init(unsigned long flags) {
    __bc_internal_flags = flags;
    if (!(bc_base_layer = (bc_base_t *) malloc(sizeof (bc_base_t)))) {
        fprintf(stderr, "Memory allocation failed.\n");
        return BC_ELAYER;
    }
#ifdef __BC_SYS_DEBUG
    if (!(bc_base_layer->mmgr = (bc_mmgr_t *) malloc(sizeof (bc_mmgr_t)))) {
        free(bc_base_layer);
        return BC_EMMGER;
    }
    bc_base_layer->mmgr->allocs = 0L;
    bc_base_layer->mmgr->frees = 0L;
#endif
    if (__bc_mpi_info_create()) {
        free(bc_base_layer);
        return BC_ENODES;
    }
    if (__bc_ltab_create()) {
        __bc_mpi_info_destroy();
        free(bc_base_layer);
        return BC_ELTAB;
    }
    if (__bc_socket_network_create()) {
        __bc_ltab_destroy();
        __bc_mpi_info_destroy();
        free(bc_base_layer);
        return BC_ESNET;
    }
    if (__bc_sthreads_create()) {
        __bc_ltab_destroy();
        __bc_mpi_info_destroy();
        free(bc_base_layer);
        return BC_ETHREADS;
    }
    if (__bc_internal_flags)
        if (__bc_optional_plists_create()) {
            __bc_ltab_destroy();
            __bc_mpi_info_destroy();
            free(bc_base_layer);
            return BC_EIPLIST;
        }
    return BC_SUCCESS;
}

/*
 * Finalise the Data System Farmework Model.
 * This should be invoked once the library is not necessary.
 */
int bc_final(void) {
    __bc_socket_network_destroy();
    __bc_ltab_destroy();

    if (__bc_internal_flags)
        __bc_optional_plists_destroy();

    __bc_mpi_info_destroy();
#ifdef __BC_SYS_DEBUG
    fprintf(stderr, "Allocs: %ld, Frees: %ld, Live: %ld\n",
            bc_base_layer->mmgr->allocs, bc_base_layer->mmgr->frees,
            bc_base_layer->mmgr->allocs - bc_base_layer->mmgr->frees);
    free(bc_base_layer->mmgr);
#endif
    free(bc_base_layer);

    return BC_SUCCESS;
}
