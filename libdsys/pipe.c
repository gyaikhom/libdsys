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
#include "pipe.h"
#include "queue.h"
#include "service.h"
#include "sockets.h"
#include "threads.h"

bc_channel_t *__bc_c_pipe(bc_plist_t *pl, bc_dtype_t *dtype,
        unsigned int du) {
    bc_channel_t *c;

    if (!(c = bc_malloc(bc_channel_t, 1)))
        return NULL;
    if (!(c->q = (bc_queue_t *)
            bc_qiface[__BC_QTYPE_MC121].c(pl, du, dtype->bytes))) {
        bc_free(c);
        return NULL;
    }
    c->role = BC_ROLE_PIPE;
    __bc_internal_dtype_ref(dtype);
    c->dtype = dtype;
    c->qtype = __BC_QTYPE_MC121;
    return c;
}

int __bc_d_pipe(bc_channel_t *c) {
    bc_qiface[__BC_QTYPE_MC121].d(c->q);
    __bc_internal_dtype_destroy(c->dtype);
    bc_free(c);
    return 0;
}

int __bc_p_pipe(bc_channel_t *c, void *local, unsigned int du) {
    return bc_qiface[__BC_QTYPE_MC121].p
            (c->q, local, c->dtype->bytes*du, 0);
}

int __bc_s_pipe(bc_channel_t *c, int fd, unsigned int du, int tag,
        int cond_recv) {
    return bc_qiface[__BC_QTYPE_MC121].s
            (c->q, fd, c->dtype->bytes*du, 0, cond_recv);
}

int __bc_r_pipe(bc_chan_t *bc, void *local, unsigned int du) {
    __bc_request_send(__bc_rank2fd_req(bc->plist->plist[0]), __BC_SERVICE_GET,
            bc->tags[0], du);

    __bc_read_data(__bc_rank2fd_data(bc->plist->plist[0]), local,
            bc->channel->dtype->bytes * du);

    return 0;
}

bc_channel_t *__bc_c_pipen(bc_plist_t *pl, bc_dtype_t *dtype,
        unsigned int du) {
    bc_channel_t *c;

    if (!(c = bc_malloc(bc_channel_t, 1)))
        return NULL;
    if (!(c->q = (bc_queue_t *)
            bc_qiface[__BC_QTYPE_NMC12M].c(pl, du, dtype->bytes))) {
        bc_free(c);
        return NULL;
    }
    c->role = BC_ROLE_PIPEN;
    __bc_internal_dtype_ref(dtype);
    c->dtype = dtype;
    c->qtype = __BC_QTYPE_NMC12M;
    return c;
}

int __bc_p_pipen(bc_channel_t *c, void *local, unsigned int du) {
    return bc_qiface[__BC_QTYPE_NMC12M].p
            (c->q, local, c->dtype->bytes*du, 1);
}

int __bc_d_pipen(bc_channel_t *c) {
    bc_qiface[__BC_QTYPE_NMC12M].d(c->q);
    __bc_internal_dtype_destroy(c->dtype);
    bc_free(c);
    return 0;
}

int __bc_s_pipen(bc_channel_t *c, int fd, unsigned int du, bc_vptr_t *vptr,
        int cond_recv) {
    return bc_qiface[__BC_QTYPE_NMC12M].sn
            (c->q, fd, c->dtype->bytes*du, vptr, cond_recv);
}
