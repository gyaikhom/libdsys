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
#include "fdutils.h"
#include "mem.h"
#include "unique_shared.h"
#include "queue.h"
#include "service.h"
#include "sockets.h"
#include "threads.h"

bc_channel_t *__bc_c_uniquely_shared(bc_plist_t *pl, bc_dtype_t *dtype,
        unsigned int du) {
    bc_channel_t *c;

    if (!(c = bc_malloc(bc_channel_t, 1)))
        return NULL;
    if (!(c->q = (bc_queue_t *)
            bc_qiface[__BC_QTYPE_UNIQUELY_SHARED].c(pl, du, dtype->bytes))) {
        bc_free(c);
        return NULL;
    }
    c->btype = BC_PATTERN_UNIQUELY_SHARED;
    __bc_internal_dtype_ref(dtype);
    c->dtype = dtype;
    c->qtype = __BC_QTYPE_UNIQUELY_SHARED;
    c->rc = pl->count;
    return c;
}

int __bc_d_uniquely_shared(bc_channel_t *c) {
    c->rc--;
    if (!c->rc) {
        bc_qiface[__BC_QTYPE_UNIQUELY_SHARED].d(c->q);
        __bc_internal_dtype_destroy(c->dtype);
        bc_free(c);
    }
    return 0;
}

int __bc_p_uniquely_shared(bc_channel_t *c, void *local, unsigned int du) {
    bc_qiface[__BC_QTYPE_UNIQUELY_SHARED].p
            (c->q, local, c->dtype->bytes*du, c->rc);
    return 0;
}

int __bc_s_uniquely_shared(bc_channel_t *c, int fd, unsigned int du,
        bc_vptr_t *vptr) {
    return bc_qiface[__BC_QTYPE_UNIQUELY_SHARED].sn(c->q, fd,
            c->dtype->bytes*du, vptr);
}
