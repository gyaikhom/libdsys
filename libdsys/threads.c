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
#include "common.h"
#include "iface.h"
#include "mem.h"
#include "service.h"
#include "threads.h"
#include <sys/socket.h>
#include <unistd.h>

static void *__bc_thread_manager(void *arg);
static void *__bc_service_thread(void *arg);

int __bc_sthreads_create(void) {
    if (!(bc_base_layer->tmgr = bc_malloc(bc_tmgr_t, 1)))
        return -1;

    bc_mutex_init(&(bc_base_layer->tmgr->lock));
    bc_cond_init(&(bc_base_layer->tmgr->cond));

    return bc_thread_create(&(bc_base_layer->tmgr->mgr),
            __bc_thread_manager, NULL);
}

void *__bc_thread_manager(void *arg) {
    long i;
    bc_thread_t t;

    bc_base_layer->tmgr->nst = bc_base_layer->mpi->size - 1;
    bc_mutex_lock(&(bc_base_layer->tmgr->lock));
    for (i = 0; i < bc_base_layer->tmgr->nst; i++)
        bc_thread_create(&t, __bc_service_thread, (void *) i);

    /* Wait for the service threads to complete. */
    bc_cond_wait(&(bc_base_layer->tmgr->cond),
            &(bc_base_layer->tmgr->lock));
    bc_mutex_unlock(&(bc_base_layer->tmgr->lock));
    bc_thread_exit(NULL);
    return NULL;
}

void *__bc_service_thread(void *arg) {
    long idx; /* Index in socket list. */
    int remote; /* Remote node being served. */
    bc_request_t r; /* Request. */
    bc_link_t *clink; /* Cached link. */
    unsigned long ctag; /* Cached tag. */
    size_t bytes; /* Bytes read in request. */
    int cond_recv; /* Is this a conditional receive? */

    idx = (long) arg;
    remote = (idx >= bc_rank) ? idx + 1 : idx;

    clink = NULL;
    ctag = 9999;

    while (1) {
        bytes = __bc_request_recv(bc_base_layer->snet->slist[idx].r_fd, &r);

        /* Update resources before exit. */
        if (bytes == 0) {
            /* Shutdown my side of the sockets. */
            shutdown(bc_base_layer->snet->slist[idx].r_fd, SHUT_RD);
            shutdown(bc_base_layer->snet->slist[idx].d_fd, SHUT_WR);

            /* Update number of data serving threads, and exit. */
            bc_mutex_lock(&(bc_base_layer->tmgr->lock));
            if (!(--bc_base_layer->tmgr->nst))
                bc_cond_signal(&(bc_base_layer->tmgr->cond));
            bc_mutex_unlock(&(bc_base_layer->tmgr->lock));
            bc_thread_exit(NULL);
        } else { /* Service request. */
            cond_recv = 0; /* Assume it is not a conditional receive. */
            switch (r.code) {
                case __BC_SERVICE_COND_GET:
                    cond_recv = 1;
                case __BC_SERVICE_GET:
                    /* Check if cached value is valid. */
                    if (clink && (r.tag == ctag))
                        /* Cache hit. */;
                    else {
                        /* Cache miss. */
                        clink = __bc_link_search(remote, r.tag);
                        ctag = r.tag;
                    }
                    switch (clink->channel->qtype) {
                        case __BC_QTYPE_MC121:
                            bc_iface[clink->channel->role].s
                                    (clink->channel,
                                    bc_base_layer->snet->slist[idx].d_fd,
                                    r.dunits, 0, cond_recv);
                            break;
                        case __BC_QTYPE_MC12M:
                            bc_iface[clink->channel->role].s
                                    (clink->channel,
                                    bc_base_layer->snet->slist[idx].d_fd,
                                    r.dunits, remote, cond_recv);
                            break;
                        case __BC_QTYPE_NMC121:
                            bc_iface[clink->channel->role].sn
                                    (clink->channel,
                                    bc_base_layer->snet->slist[idx].d_fd,
                                    r.dunits, &(clink->vptr), cond_recv);
                            break;
                        case __BC_QTYPE_NMC12M:
                            bc_iface[clink->channel->role].sn
                                    (clink->channel,
                                    bc_base_layer->snet->slist[idx].d_fd,
                                    r.dunits, &(clink->vptr), cond_recv);
                            break;
                        case __BC_QTYPE_FARMN:
                            bc_iface[clink->channel->role].sn
                                    (clink->channel,
                                    bc_base_layer->snet->slist[idx].d_fd,
                                    r.dunits, NULL, cond_recv);
                        case __BC_QTYPE_CUST:
                        default:
                            break;
                    }
                    break;

                case __BC_SERVICE_KILL:
                    __bc_link_destroy(remote, r.tag);
                    clink = NULL; /* Invalidate cache. */
                    break;
                case __BC_SERVICE_ECHO:
                    fprintf(stderr, "[%d:%d] Hello from %d\n",
                            bc_base_layer->mpi->rank, remote, remote);
                    break;
                case __BC_SERVICE_NUM:
                    break;
            }
        }
    }
    return NULL;
}
