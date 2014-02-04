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
#include "queue.h"
#include "replicate.h"
#include "service.h"
#include "sockets.h"
#include "threads.h"

bc_channel_t *__bc_c_replicate(bc_plist_t *pl, bc_dtype_t *dtype,
        unsigned int du) {
    bc_channel_t *c;

    if (!(c = bc_malloc(bc_channel_t, 1)))
        return NULL;

    if (!(c->q = (bc_queue_t *)
            bc_qiface[__BC_QTYPE_MC12M].c(pl, du, dtype->bytes))) {
        bc_free(c);
        return NULL;
    }
    c->role = BC_ROLE_REPLICATE;
    __bc_internal_dtype_ref(dtype);
    c->dtype = dtype;
    c->qtype = __BC_QTYPE_MC12M;
    c->rc = pl->count;
    return c;
}

int __bc_d_replicate(bc_channel_t *c) {
    c->rc--;
    if (!(c->rc)) {
        bc_qiface[__BC_QTYPE_MC12M].d(c->q);
        __bc_internal_dtype_destroy(c->dtype);
        bc_free(c);
    }
    return 0;
}

int __bc_p_replicate(bc_channel_t *c, void *local, unsigned int du) {
    size_t s;
    bc_mc12m_t *temp;

    temp = c->q->mc12m;
    s = c->dtype->bytes*du;
    while (temp) {
        bc_qiface[__BC_QTYPE_MC12M].p(c->q, local, s, temp->tag);
        temp = temp->next;
    }
    return 0;
}

int __bc_s_replicate(bc_channel_t *c, int fd, unsigned int du, int tag,
        int cond_recv) {
    return bc_qiface[__BC_QTYPE_MC12M].s(c->q, fd, c->dtype->bytes*du, tag,
            cond_recv);
}

bc_channel_t *__bc_c_replicaten(bc_plist_t *pl, bc_dtype_t *dtype,
        unsigned int du) {
    bc_channel_t *c;

    if (!(c = bc_malloc(bc_channel_t, 1)))
        return NULL;
    if (!(c->q = (bc_queue_t *)
            bc_qiface[__BC_QTYPE_NMC12M].c(pl, du, dtype->bytes))) {
        bc_free(c);
        return NULL;
    }
    c->role = BC_ROLE_REPLICATEN;
    __bc_internal_dtype_ref(dtype);
    c->dtype = dtype;
    c->qtype = __BC_QTYPE_NMC12M;
    c->rc = pl->count;
    return c;
}

int __bc_d_replicaten(bc_channel_t *c) {
    c->rc--;
    if (!c->rc) {
        bc_qiface[__BC_QTYPE_NMC12M].d(c->q);
        __bc_internal_dtype_destroy(c->dtype);
        bc_free(c);
    }
    return 0;
}

int __bc_p_replicaten(bc_channel_t *c, void *local, unsigned int du) {
    bc_qiface[__BC_QTYPE_NMC12M].p
            (c->q, local, c->dtype->bytes*du, c->rc);
    return 0;
}

int __bc_s_replicaten(bc_channel_t *c, int fd, unsigned int du,
        bc_vptr_t *vptr, int cond_recv) {
    return bc_qiface[__BC_QTYPE_NMC12M].sn(c->q, fd, c->dtype->bytes*du,
            vptr, cond_recv);
}
