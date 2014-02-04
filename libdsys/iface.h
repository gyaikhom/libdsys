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
#error "Please do not include 'iface.h'; include 'dsys.h' instead."
#endif

#ifndef _BC_IFACE_H
#define _BC_IFACE_H

#include "branch.h"
#include "common.h"
#include "dtype.h"
#include "plist.h"
#include "queue.h"

BEGIN_C_DECLS

typedef bc_channel_t *(*bc_c_t)(bc_plist_t *pl, bc_dtype_t *dt,
        unsigned int du);
typedef int (*bc_d_t)(bc_channel_t *c);
typedef int (*bc_p_t)(bc_channel_t *c, void *local, unsigned int du);
typedef int (*bc_s_t)(bc_channel_t *c, int fd, unsigned int du,
        int tag, int cond_recv);
typedef int (*bc_sn_t)(bc_channel_t *c, int fd, unsigned int du,
        bc_vptr_t *vptr, int cond_recv);
typedef int (*bc_r_t)(bc_chan_t *bc, void *local, unsigned int du);
typedef void (*bc_reduce_opt_t) (void *buffer, int offset, int count);

typedef struct {
    bc_c_t c; /* Create buffer. */
    bc_d_t d; /* Destroy buffer. */
    bc_p_t p; /* Put into buffer. */
    bc_s_t s; /* Send from buffer. */
    bc_sn_t sn; /* Send from buffer. */
    bc_r_t r; /* Remote get. */
} bc_iface_t;

extern bc_iface_t const bc_iface[];
extern bc_reduce_opt_t bc_reduce_operator;

END_C_DECLS

#endif /* _BC_IFACE_H */
