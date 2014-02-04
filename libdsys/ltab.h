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
#error "Please do not include 'ltab.h'; include 'dsys.h' instead."
#endif

#ifndef _BC_LTAB_H
#define _BC_LTAB_H

#include "branch.h"
#include "common.h"
#include "threads.h"
#include "queue.h"

/*
 * Separate tags are maintained for creation of source
 * and sink branching channels. The following macros are
 * used to get and update the next link tag.
 */
#define __bc_gtag(rank) \
    (bc_base_layer->ltab[(rank)].src_tag++)
#define __bc_ptag(rank) \
    (bc_base_layer->ltab[(rank)].sink_tag++)

BEGIN_C_DECLS

/*
 * The lookup table is managed as a hash table. For all the processes
 * in the topology, hooks are created. These hooks are the head of the link
 * list created by inserting links in LIFO.
 */
typedef struct bc_link_s bc_link_t;

struct bc_link_s {
    unsigned long tag; /* Unique source or sink link tag. */
    bc_link_t *next; /* Pointer to next link. */
    bc_channel_t *channel; /* Pointer to channel. */
    bc_vptr_t vptr; /* variable pointer. */
};

typedef struct bc_hook_s bc_hook_t;

struct bc_hook_s {
    bc_link_t dummy; /* Dummy node, for fine grain locking. */
    bc_mutex_t lock; /* Lock for modifying list of links. */
    bc_cond_t wait; /* Wait for link creation. */
    unsigned long src_tag; /* Current source tag. */
    unsigned long sink_tag; /* Current sink tag. */
};

extern int __bc_ltab_create(void);
extern int __bc_ltab_destroy(void);
extern int __bc_link_insert(const bc_chan_t *bc);
extern bc_link_t *__bc_link_search(int rank, unsigned long tag);
extern int __bc_link_destroy(int rank, unsigned long tag);

END_C_DECLS

#endif /* _BC_LTAB_H */
