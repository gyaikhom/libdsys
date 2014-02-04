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
#include "queue.h"
#include "reduce.h"
#include "service.h"
#include "sockets.h"
#include "variable.h"

int __bc_r_sum_reduce(bc_chan_t *bc, void *local, unsigned int du) {
    register int i, j;
    void *temp, *out_ptr = local, *temp_ptr;
    size_t sz = bc->channel->dtype->bytes*du;

    if (!(temp = (void *) bc_malloc(char, sz)))
        return -1;

    /*
     * Send request for bulk transfer of all the data units
     * required for the communication.
     */
    for (i = 0; i < bc->plist->count; i++)
        __bc_request_send(__bc_rank2fd_req(bc->plist->plist[i]),
            __BC_SERVICE_GET, bc->tags[i], du);

    /*
     * Initialize the values in the local variable to 0 so that
     * we can start adding once data starts coming.
     */
    for (i = 0; i < du; i++) {
        bc_variable_assign(bc->channel->dtype, out_ptr, (long double) 0);
        out_ptr += bc->channel->dtype->bytes;
    }

    /*
     * Read data from the producer and do the summation.
     */
    for (i = 0; i < bc->plist->count; i++) {
        __bc_read_data(__bc_rank2fd_data(bc->plist->plist[i]),
                temp, sz);

        out_ptr = local;
        temp_ptr = temp;
        for (j = 0; j < du; j++) {
            bc_variable_operation(bc->channel->dtype, BC_VARIABLE_ADD,
                    out_ptr, out_ptr, temp_ptr);
            out_ptr += bc->channel->dtype->bytes;
            temp_ptr += bc->channel->dtype->bytes;
        }
    }

    bc_free(temp);
    return 0;
}

int __bc_r_mul_reduce(bc_chan_t *bc, void *local, unsigned int du) {
    register int i, j;
    void *temp, *out_ptr = local, *temp_ptr;
    size_t sz = bc->channel->dtype->bytes * du;

    if (!(temp = (void *) bc_malloc(char, sz)))
        return -1;

    for (i = 0; i < bc->plist->count; i++)
        __bc_request_send(__bc_rank2fd_req(bc->plist->plist[i]),
            __BC_SERVICE_GET, bc->tags[i], du);

    for (i = 0; i < du; i++) {
        bc_variable_assign(bc->channel->dtype, out_ptr, (long double) 1);
        out_ptr += bc->channel->dtype->bytes;
    }

    for (i = 0; i < bc->plist->count; i++) {
        __bc_read_data(__bc_rank2fd_data(bc->plist->plist[i]), temp, sz);

        out_ptr = local;
        temp_ptr = temp;
        for (j = 0; j < du; j++) {
            bc_variable_operation(bc->channel->dtype, BC_VARIABLE_MULTIPLY,
                    out_ptr, out_ptr, temp_ptr);
            out_ptr += bc->channel->dtype->bytes;
            temp_ptr += bc->channel->dtype->bytes;
        }
    }

    bc_free(temp);
    return 0;
}

int __bc_r_max_reduce(bc_chan_t *bc, void *local, unsigned int du) {
    register int i, j;
    void *temp, *out_ptr = local, *temp_ptr;
    size_t sz = bc->channel->dtype->bytes * du;

    if (!(temp = (void *) bc_malloc(char, sz)))
        return -1;

    for (i = 0; i < bc->plist->count; i++)
        __bc_request_send(__bc_rank2fd_req(bc->plist->plist[i]),
            __BC_SERVICE_GET, bc->tags[i], du);

    __bc_read_data(__bc_rank2fd_data(bc->plist->plist[0]), local, sz);

    for (i = 1; i < bc->plist->count; i++) {
        __bc_read_data(__bc_rank2fd_data(bc->plist->plist[i]), temp, sz);

        out_ptr = local;
        temp_ptr = temp;
        for (j = 0; j < du; j++) {
            bc_variable_operation(bc->channel->dtype, BC_VARIABLE_MAX,
                    out_ptr, out_ptr, temp_ptr);
            out_ptr += bc->channel->dtype->bytes;
            temp_ptr += bc->channel->dtype->bytes;
        }
    }

    bc_free(temp);
    return 0;
}

int __bc_r_min_reduce(bc_chan_t *bc, void *local, unsigned int du) {
    register int i, j;
    void *temp, *out_ptr = local, *temp_ptr;
    size_t sz = bc->channel->dtype->bytes * du;

    if (!(temp = (void *) bc_malloc(char, sz)))
        return -1;

    for (i = 0; i < bc->plist->count; i++)
        __bc_request_send(__bc_rank2fd_req(bc->plist->plist[i]),
            __BC_SERVICE_GET, bc->tags[i], du);

    __bc_read_data(__bc_rank2fd_data(bc->plist->plist[0]), local, sz);

    for (i = 1; i < bc->plist->count; i++) {
        __bc_read_data(__bc_rank2fd_data(bc->plist->plist[i]), temp, sz);

        out_ptr = local;
        temp_ptr = temp;
        for (j = 0; j < du; j++) {
            bc_variable_operation(bc->channel->dtype, BC_VARIABLE_MIN,
                    out_ptr, out_ptr, temp_ptr);
            out_ptr += bc->channel->dtype->bytes;
            temp_ptr += bc->channel->dtype->bytes;
        }
    }

    bc_free(temp);
    return 0;
}

int __bc_r_minmax_reduce(bc_chan_t *bc, void *local, unsigned int du) {
    register int i, j;
    void *temp, *out_ptr = local, *temp_ptr;
    size_t sz = bc->channel->dtype->bytes * du;

    if (!(temp = (void *) bc_malloc(char, sz)))
        return -1;

    for (i = 0; i < bc->plist->count; i++)
        __bc_request_send(__bc_rank2fd_req(bc->plist->plist[i]),
            __BC_SERVICE_GET, bc->tags[i], du);

    __bc_read_data(__bc_rank2fd_data(bc->plist->plist[0]), local, sz);
    memcpy(local + sz, local, sz);

    for (i = 1; i < bc->plist->count; i++) {
        __bc_read_data(__bc_rank2fd_data(bc->plist->plist[i]), temp, sz);

        out_ptr = local;
        temp_ptr = temp;
        for (j = 0; j < du; j++) {
            bc_variable_operation(bc->channel->dtype, BC_VARIABLE_MIN,
                    out_ptr, out_ptr, temp_ptr);
            out_ptr += bc->channel->dtype->bytes;
            temp_ptr += bc->channel->dtype->bytes;
        }

        out_ptr = local + sz;
        temp_ptr = temp;
        for (j = 0; j < du; j++) {
            bc_variable_operation(bc->channel->dtype, BC_VARIABLE_MAX,
                    out_ptr, out_ptr, temp_ptr);
            out_ptr += bc->channel->dtype->bytes;
            temp_ptr += bc->channel->dtype->bytes;
        }
    }

    bc_free(temp);
    return 0;
}

int __bc_r_opt_reduce(bc_chan_t *bc, void *local, unsigned int du) {
    register int i;
    void *temp, *out_ptr = local, *temp_ptr;
    size_t sz = bc->channel->dtype->bytes * du;

    if (!(temp = (void *) bc_malloc(char, sz * bc->plist->count)))
        return -1;

    for (i = 0; i < bc->plist->count; i++)
        __bc_request_send(__bc_rank2fd_req(bc->plist->plist[i]),
            __BC_SERVICE_GET, bc->tags[i], du);

    temp_ptr = temp;
    for (i = 0; i < bc->plist->count; i++) {
        __bc_read_data(__bc_rank2fd_data(bc->plist->plist[i]), temp_ptr, sz);
        temp_ptr += sz;
    }

    for (i = 0; i < du; i++) {
        bc_reduce_operator(temp, bc->channel->dtype->bytes, bc->plist->count);
        memcpy(out_ptr, temp, bc->channel->dtype->bytes);
        out_ptr += bc->channel->dtype->bytes;
        temp += bc->channel->dtype->bytes;
    }

    bc_free(temp);
    return 0;
}
