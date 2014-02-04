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
#include "collect.h"
#include "service.h"
#include "sockets.h"
#include "variable.h"

int __bc_r_collect(bc_chan_t *bc, void *local, unsigned int du) {
    register int i;
    void *out_ptr = local;
    size_t sz = bc->channel->dtype->bytes*du;

    for (i = 0; i < bc->plist->count; i++)
        __bc_request_send(__bc_rank2fd_req(bc->plist->plist[i]),
            __BC_SERVICE_GET, bc->tags[i], du);

    for (i = 0; i < bc->plist->count; i++) {
        __bc_read_data(__bc_rank2fd_data(bc->plist->plist[i]),
                out_ptr, sz);
        out_ptr += sz;
    }

    return 0;
}
