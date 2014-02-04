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
#error "Please do not include 'sockets.h'; include 'dsys.h' instead."
#endif

#ifndef _BC_SOCKETS_H
#define _BC_SOCKETS_H

#include "common.h"

/*
 * Base port number + rank is used for creating listener socket.
 */
#define __BC_BASE_PORT_NUMBER (6666)

#define __bc_rank2fd_req(X) \
        (((X) > bc_base_layer->mpi->rank) ? \
        bc_base_layer->snet->slist[(X)-1].r_fd : \
        bc_base_layer->snet->slist[(X)].r_fd)

#define __bc_rank2fd_data(X) \
        (((X) > bc_base_layer->mpi->rank) ? \
        bc_base_layer->snet->slist[(X)-1].d_fd : \
        bc_base_layer->snet->slist[(X)].d_fd)

#define __bc_rank2idx(X) \
        (((X) > bc_base_layer->mpi->rank) ? \
        (X)-1 : (X))

#define __bc_idx2fd_req(X) \
        (bc_base_layer->snet->slist[(X)].r_fd)

#define __bc_idx2fd_data(X) \
        (bc_base_layer->snet->slist[(X)].d_fd)

BEGIN_C_DECLS

enum {
    REQ_SOCK = 0,
    DATA_SOCK
};

typedef struct bc_sockpair_s {
    int r_fd; /* request file descriptor. */
    int d_fd; /* data transfer file descriptor. */
} bc_sockpair_t;

typedef struct bc_snet_s {
    unsigned int count; /* Number of pairs in the list. */
    bc_sockpair_t *slist; /* Socket list. */
} bc_snet_t;

extern int __bc_socket_network_create(void);
extern int __bc_socket_network_destroy(void);

END_C_DECLS

#endif /* _BC_SOCKETS_H */
