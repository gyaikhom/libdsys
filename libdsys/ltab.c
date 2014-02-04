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
#include "iface.h"
#include "ltab.h"
#include "mem.h"

int __bc_ltab_create(void) {
    register int i;

    if (!(bc_base_layer->ltab =
            bc_malloc(bc_hook_t, bc_base_layer->mpi->size))) {
        return -1;
    }
    for (i = 0; i < bc_base_layer->mpi->size; i++) {
        bc_base_layer->ltab[i].dummy.next = NULL;
        bc_mutex_init(&(bc_base_layer->ltab[i].lock));
        bc_cond_init(&(bc_base_layer->ltab[i].wait));
        bc_base_layer->ltab[i].src_tag = 0L;
        bc_base_layer->ltab[i].sink_tag = 0L;
    }
    return 0;
}

int __bc_ltab_destroy(void) {
    register int i;
    bc_link_t *temp;

    /* 
     * Lookup table is destroyed when no data serving threads
     * are alive, hence, we need not lock before destroying
     * the list of links. Locks are used only when data serving
     * threads are alive.
     */
    for (i = 0; i < bc_base_layer->mpi->size; i++) {
        while (bc_base_layer->ltab[i].dummy.next) {
            /* Update linked list for head deletion. */
            temp = bc_base_layer->ltab[i].dummy.next;
            bc_base_layer->ltab[i].dummy.next = temp->next;

            /* Destroy buffer associated with the previous root. */
            bc_iface[temp->channel->role].d(temp->channel);
            bc_free(temp);
        }
    }

    bc_free(bc_base_layer->ltab);
    return 0;
}

int __bc_link_insert(const bc_chan_t *bc) {
    register int i, rank;
    bc_link_t *link;

    for (i = 0; i < bc->plist->count; i++)
        if (!(link = bc_malloc(bc_link_t, 1)))
            return -1;
        else {
            link->tag = bc->tags[i];
            link->channel = bc->channel;

            switch (bc->channel->qtype) {
                case __BC_QTYPE_NMC121:
                    link->vptr.var = bc->channel->q->nmc121->start;
                    break;
                case __BC_QTYPE_NMC12M:
                    link->vptr.var = bc->channel->q->nmc12m->start;
                    link->vptr.rcidx = 0;
                    break;
                default:
                    break;
            }

            /*
             * Don't allow access to the linked list dummy 
             * during insertion.
             */
            rank = bc->plist->plist[i];
            bc_mutex_lock(&(bc_base_layer->ltab[rank].lock));
            {
                link->next = bc_base_layer->ltab[rank].dummy.next;
                bc_base_layer->ltab[rank].dummy.next = link;
            }
            bc_cond_signal(&(bc_base_layer->ltab[rank].wait));
            bc_mutex_unlock(&(bc_base_layer->ltab[rank].lock));
        }
    return 0;
}

bc_link_t *__bc_link_search(int rank, unsigned long tag) {
    bc_link_t *temp, *dummy;

    bc_mutex_lock(&(bc_base_layer->ltab[rank].lock));
    do {
        /* Don't allow changes to the root of the link list. */
        temp = bc_base_layer->ltab[rank].dummy.next;
        dummy = temp;
        bc_mutex_unlock(&(bc_base_layer->ltab[rank].lock));

        /* Search for the link. */
        while (temp)
            if (temp->tag == tag)
                return temp;
            else
                temp = temp->next;

        /* Link not found, check if the root has since changed. */
        bc_mutex_lock(&(bc_base_layer->ltab[rank].lock));
        if (dummy != bc_base_layer->ltab[rank].dummy.next)
            continue;

        bc_cond_wait(&(bc_base_layer->ltab[rank].wait),
                &(bc_base_layer->ltab[rank].lock));
    } while (1);
}

int __bc_link_destroy(int rank, unsigned long tag) {
    bc_link_t *rtemp, *temp, *dummy;

    bc_mutex_lock(&(bc_base_layer->ltab[rank].lock));

again:
    rtemp = bc_base_layer->ltab[rank].dummy.next;
    bc_mutex_unlock(&(bc_base_layer->ltab[rank].lock));

    /*
     * The locking is required only if the root is the link
     * to be destroyed. This is because insertion only affects
     * the root. For all the internal links, we can do without
     * locking.
     */
    if (rtemp->tag == tag) {
        temp = rtemp->next;

        /* Destroy the process list if I am the last one depending. */
        bc_iface[rtemp->channel->role].d(rtemp->channel);
        bc_free(rtemp);

        /* Check if root is still valid. */
        bc_mutex_lock(&(bc_base_layer->ltab[rank].lock));
        if (rtemp == bc_base_layer->ltab[rank].dummy.next)
            bc_base_layer->ltab[rank].dummy.next = temp;
        else {
            dummy = bc_base_layer->ltab[rank].dummy.next;

            while (dummy->next != rtemp)
                dummy = dummy->next;
            dummy->next = temp;
        }
        bc_mutex_unlock(&(bc_base_layer->ltab[rank].lock));
        return 0;
    } else {
        temp = rtemp;

        /* Search for internal link. */
        while (temp->next->tag != tag) {
            temp = temp->next;
            if (!temp->next)
                break;
        }

        if (!temp->next) {
            bc_mutex_lock(&(bc_base_layer->ltab[rank].lock));
            if (rtemp != bc_base_layer->ltab[rank].dummy.next)
                goto again;

            bc_cond_wait(&(bc_base_layer->ltab[rank].wait),
                    &(bc_base_layer->ltab[rank].lock));
            goto again;
        }

        dummy = temp->next;
        temp->next = dummy->next;
        bc_iface[temp->channel->role].d(dummy->channel);
        bc_free(dummy);
        bc_mutex_unlock(&(bc_base_layer->ltab[rank].lock));
        return 0;
    }

    return -1;
}
