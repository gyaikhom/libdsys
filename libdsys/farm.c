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
#include "fdutils.h"
#include "mem.h"
#include "farm.h"
#include "queue.h"
#include "service.h"
#include "sockets.h"
#include "threads.h"

bc_channel_t *__bc_c_farmn(bc_plist_t *pl, bc_dtype_t *dtype,
        unsigned int du) {
    bc_channel_t *c;

    if (!(c = bc_malloc(bc_channel_t, 1)))
        return NULL;

    if (!(c->q = (bc_queue_t *)
            bc_qiface[__BC_QTYPE_FARMN].c(pl, du, dtype->bytes))) {
        bc_free(c);
        return NULL;
    }
    c->role = BC_ROLE_FARMN;
    __bc_internal_dtype_ref(dtype);
    c->dtype = dtype;
    c->qtype = __BC_QTYPE_FARMN;
    c->rc = pl->count;
    return c;
}

int __bc_d_farmn(bc_channel_t *c) {
    c->rc--;
    if (!c->rc) {
        bc_qiface[__BC_QTYPE_FARMN].d(c->q);
        __bc_internal_dtype_destroy(c->dtype);
        bc_free(c);
    }
    return 0;
}

int __bc_p_farmn(bc_channel_t *c, void *local, unsigned int du) {
    bc_qiface[__BC_QTYPE_FARMN].p
            (c->q, local, c->dtype->bytes*du, c->rc);
    return 0;
}

int __bc_s_farmn(bc_channel_t *c, int fd, unsigned int du,
        bc_vptr_t *vptr, int cond_recv) {
    return bc_qiface[__BC_QTYPE_FARMN].sn(c->q, fd, c->dtype->bytes*du,
            vptr, cond_recv);
}
