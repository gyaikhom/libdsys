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

#ifndef __BC_SYS_COMPILE
#error "Please do not include 'base.h'; include 'dsys.h' instead."
#endif

#ifndef _BC_BASE_H
#define _BC_BASE_H

#include "common.h"
#include "infmpi.h"
#include "ltab.h"
#include "mem.h"
#include "sockets.h"
#include "threads.h"

BEGIN_C_DECLS

typedef struct bc_base_s {
    bc_hook_t *ltab; /* Lookup table for links. */
    bc_snet_t *snet; /* Socket network. */
    bc_tmgr_t *tmgr; /* Threads management. */
#ifdef __BC_SYS_DEBUG
    bc_mmgr_t *mmgr; /* Memory manager. */
#endif
    bc_mpi_t *mpi; /* MPI information. */
} bc_base_t;

extern bc_base_t *bc_base_layer;
extern int bc_init(unsigned long flags);
extern int bc_final(void);

END_C_DECLS

#endif /* _BC_BASE_H */
