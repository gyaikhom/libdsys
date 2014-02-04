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

#include "mem.h"
#include "base.h"

#ifdef __BC_SYS_DEBUG

void *__bc__malloc(size_t size, unsigned long line, const char *fname) {
    bc_base_layer->mmgr->allocs++;
#ifdef __BC_SYS_MTRACE
    fprintf(stderr, "[%d] %s:%ld: (alloc) Allocs: %ld, Frees: %ld, Live: %ld\n",
            bc_base_layer->mpi ? bc_base_layer->mpi->rank : -1, fname, line,
            bc_base_layer->mmgr->allocs, bc_base_layer->mmgr->frees,
            bc_base_layer->mmgr->allocs - bc_base_layer->mmgr->frees);
#endif
    return malloc(size);
}

void __bc__free(void *ptr, unsigned long line, const char *fname) {
    free(ptr);
    bc_base_layer->mmgr->frees++;
#ifdef __BC_SYS_MTRACE
    fprintf(stderr, "[%d] %s:%ld: (free) Allocs: %ld, Frees: %ld, Live: %ld\n",
            bc_base_layer->mpi ? bc_base_layer->mpi->rank : -1, fname, line,
            bc_base_layer->mmgr->allocs, bc_base_layer->mmgr->frees,
            bc_base_layer->mmgr->allocs - bc_base_layer->mmgr->frees);
#endif
}
#endif
