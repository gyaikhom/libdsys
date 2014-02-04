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
#include "branch.h"
#include "iface.h"
#include "ltab.h"
#include "mem.h"
#include "service.h"
#include "sockets.h"
#include <stdarg.h>

#ifdef BC_SHOW_ERROR
bc_chan_t *__bc_chan_create(bc_plist_t *prod, bc_plist_t *cons,
        bc_dtype_t *dtype, unsigned int bunits, bc_role_t role,
        unsigned long line, const char *fname)
#else

bc_chan_t *bc_chan_create(bc_plist_t *prod, bc_plist_t *cons,
        bc_dtype_t *dtype, unsigned int bunits, bc_role_t role)
#endif
{
#ifdef BC_SHOW_ERROR
    if (prod)
        return __bc_src_create(prod, dtype, role, line, fname);
    else
        return __bc_sink_create(cons, dtype, bunits, role, line, fname);
#else
    if (prod)
        return bc_src_create(prod, dtype, role);
    else
        return bc_sink_create(cons, dtype, bunits, role);
#endif
}

#ifdef BC_SHOW_ERROR
/*
 * Create a sink branching channel.
 *
 * cons   - Consumer process lists.
 * dtype  - Branching channel data type. 
 * bunits - Buffer units required.
 * role   - Role of the branching channel.
 * line   - Line number in the application code (used for debugging).
 * fname  - Filename name of the application code.
 *
 * Returns valid branching channel, or NULL upon error.
 */
bc_chan_t *__bc_sink_create(bc_plist_t *cons, bc_dtype_t *dtype,
        unsigned int bunits, bc_role_t role, unsigned long line,
        const char *fname)
#else

bc_chan_t *bc_sink_create(bc_plist_t *cons, bc_dtype_t *dtype,
        unsigned int bunits, bc_role_t role)
#endif
{
    bc_chan_t *bc;
    register int i;

    if (!cons) {
        bc_perror(BC_EICONS, line, fname);
        return NULL;
    }
    if (!dtype) {
        bc_perror(BC_EIDTYPE, line, fname);
        return NULL;
    }
    if (role < 0 || role >= BC_ROLE_NUM) {
        bc_perror(BC_EIROLE, line, fname);
        return NULL;
    }
    if (!(bc = bc_malloc(bc_chan_t, 1))) {
        bc_perror(BC_EMEM, line, fname);
        return NULL;
    }
    if (!(bc->tags = bc_malloc(unsigned long, cons->count))) {
        bc_free(bc);
        bc_perror(BC_EMEM, line, fname);
        return NULL;
    }
    if (!(bc->channel = bc_iface[role].c(cons, dtype, bunits))) {
        bc_free(bc->tags);
        bc_free(bc);
        bc_perror(BC_EMEM, line, fname);
        return NULL;
    }
    bc->mode = __BC_PRODUCER;
    bc->plist = cons;

    /* Update reference counters. */
    __bc_internal_dtype_ref(dtype);
    __bc_internal_plist_ref(cons);

    /*
     * Set the appropriate tags corresponding to the internal links
     * created between two processes.
     *
     * Non-crossing edges:
     * In order to simplify using the API, the creation of the internal
     * links are automated. Hence, the branching channels are linked based
     * on automatically generated link tag. The tags are generated for a
     * pair of processes. Hence, the creation of more than two branching
     * channels should not cross each other if they are connected by an edge.
     * E.g. For correspondence, A = C, and B = D, 
     *
     * (VALID)
     * p0: ----A----B---------->
     *         |    |
     *         |    |
     * p1: ----C----D---------->
     *
     * tag(A) = tag(C) = 0;tag(B) = tag(D) = 1;
     *
     *
     * (INVALID)
     * p0: ----A----B---------->
     *          \  /
     *           \/
     *           /\
     *          /  \
     * p1: ----D----C---------->
     *
     * (tag(A) = 0) != (tag(C) = 1);
     * (tag(B) = 1) != (tag(D) = 0);
     * 
     * NOTE: By using a compiler, this can be resolved easily by using
     * the name space matching where two branching channels corresponds if
     * their declaration have the same name.
     */
    for (i = 0; i < cons->count; i++)
        bc->tags[i] = __bc_ptag(cons->plist[i]);

    /* Insert the branching channel into the hash table. */
    __bc_link_insert(bc);
    return bc;
}

#ifdef BC_SHOW_ERROR
/*
 * Create a source branching channel.
 *
 * prod  - Producer process lists.
 * dtype - Branching channel data type. 
 * role  - Role of the branching channel.
 * line  - Line number in the application code (used for debugging).
 * fname - Filename name of the application code.
 *
 * Returns valid branching channel, or NULL upon error.
 */
bc_chan_t *__bc_src_create(bc_plist_t *prod, bc_dtype_t *dtype,
        bc_role_t role, unsigned long line, const char *fname)
#else

bc_chan_t *bc_src_create(bc_plist_t *prod, bc_dtype_t *dtype, bc_role_t role)
#endif
{
    bc_chan_t *bc;
    register int i;

    if (!prod) {
        bc_perror(BC_EIPROD, line, fname);
        return NULL;
    }
    if (!dtype) {
        bc_perror(BC_EIDTYPE, line, fname);
        return NULL;
    }
    if (role < 0 || role >= BC_ROLE_NUM) {
        bc_perror(BC_EIROLE, line, fname);
        return NULL;
    }
    if (!(bc = bc_malloc(bc_chan_t, 1))) {
        bc_perror(BC_EMEM, line, fname);
        return NULL;
    }
    if (!(bc->tags = bc_malloc(unsigned long, prod->count))) {
        bc_free(bc);
        bc_perror(BC_EMEM, line, fname);
        return NULL;
    }
    if (!(bc->channel = bc_malloc(bc_channel_t, 1))) {
        bc_free(bc->tags);
        bc_free(bc);
        bc_perror(BC_EMEM, line, fname);
        return NULL;
    }
    bc->mode = __BC_CONSUMER;
    bc->plist = prod;
    bc->channel->role = role;
    bc->channel->dtype = dtype;
    bc->channel->q = NULL; /* No queue associated with source. */

    /* Update reference counters. */
    __bc_internal_dtype_ref(dtype);
    __bc_internal_plist_ref(prod);

    /* Set the tags. */
    for (i = 0; i < prod->count; i++)
        bc->tags[i] = __bc_gtag(prod->plist[i]);

    /*
     * Because no queue is managed, we do not enter this into the
     * hash table of internal links because only the creating processes
     * uses this branching channel.
     */
    return bc;
}

#ifdef BC_SHOW_ERROR
/*
 * Display branching channel information.
 *
 * bc    - Branching channel.
 * line  - Line number in the application code (used for debugging).
 * fname - Filename name of the application code.
 *
 * Returns BC_SUCCESS if succes, else error code.
 */
int __bc_chan_info(bc_chan_t *bc, unsigned long line, const char *fname)
#else

int bc_chan_info(bc_chan_t *bc)
#endif
{
    register int i;

    if (!bc) {
        bc_perror(BC_EIBC, line, fname);
        return BC_EIBC;
    }
    if (bc->mode == __BC_PRODUCER) {
        printf("Consumer list: ");
        for (i = 0; i < bc->plist->count; i++)
            printf("%d ", bc->plist->plist[i]);
        printf("\nTags: ");
        for (i = 0; i < bc->plist->count; i++)
            printf("%ld ", bc->tags[i]);
        printf("\nData type: %d, %ld bytes\n", bc->channel->dtype->code,
                bc->channel->dtype->bytes);
    } else {
        printf("Producer list: ");
        for (i = 0; i < bc->plist->count; i++)
            printf("%d ", bc->plist->plist[i]);
        printf("\nTags: ");
        for (i = 0; i < bc->plist->count; i++)
            printf("%ld ", bc->tags[i]);
        printf("\nData type: %d, %ld bytes\n", bc->channel->dtype->code,
                bc->channel->dtype->bytes);
    }
    return BC_SUCCESS;
}

#ifdef BC_SHOW_ERROR
/*
 * Destroys a branching channel.
 *
 * bc    - Branching channel to destroy.
 * line  - Line number in the application code (used for debugging).
 * fname - Filename name of the application code.
 *
 * Returns BC_SUCCESS if succes, else error code.
 */
int __bc_chan_destroy(bc_chan_t *bc, unsigned long line, const char *fname)
#else

int bc_chan_destroy(bc_chan_t *bc)
#endif
{
    if (!bc) {
        bc_perror(BC_EIBC, line, fname);
        return BC_EIBC;
    }
    if (bc->mode == __BC_CONSUMER) {
        register int i;

        /*
         * Send branching channel destoy service request to all
         * the producers which is linked to the calling process.
         */
        for (i = 0; i < bc->plist->count; i++)
            __bc_request_send(__bc_rank2fd_req(bc->plist->plist[i]),
                __BC_SERVICE_KILL, bc->tags[i], 0);

        /*
         * Because we don't manage a queue, we have to explicitly
         * deallocate the channel data structure which was defined
         * explicitly in '__bc_chan_src_create()'.
         */
        bc_free(bc->channel);
    }
    /* Destroy the data type if possible. */
    __bc_internal_dtype_destroy(bc->channel->dtype);

    /* Destroy the process list if possible. */
    __bc_internal_plist_destroy(bc->plist);

    /* Free the data structures for storing the tags. */
    bc_free(bc->tags);

    /* Destroy the branching channel. */
    bc_free(bc);

    return BC_SUCCESS;
}

#ifdef BC_SHOW_ERROR
/*
 * Put data into the branching channel.
 *
 * bc    - Branching channel.
 * local - Local application buffer.
 * du    - Number of data units to put into branching channel.
 * line  - Line number in the application code (used for debugging).
 * fname - Filename name of the application code.
 *
 * Returns BC_SUCCESS if succes, else error code.
 */
int __bc_put(bc_chan_t *bc, void *local, int du, unsigned long line,
        const char *fname)
#else

int bc_put(bc_chan_t *bc, void *local, int du)
#endif
{
    if (!bc) {
        bc_perror(BC_EIBC, line, fname);
        return BC_EIBC;
    }
    if (bc->mode != __BC_PRODUCER) {
        bc_perror(BC_EIFUNC, line, fname);
        return BC_EIFUNC;
    }
    if (!local) {
        bc_perror(BC_EIPTR, line, fname);
        return BC_EIPTR;
    }
    if (du < 1) {
        bc_perror(BC_EIVAL, line, fname);
        return BC_EIVAL;
    }

    /*
     * Invoke the put() interface associated with the
     * branching channel role.
     */
    return bc_iface[bc->channel->role].p(bc->channel, local, du);
}

#ifdef BC_SHOW_ERROR
/*
 * Commits one data unit into the buffer by promoting the data in the
 * local variable abstraction buffer unit to a system buffer unit.
 *
 * bc    - Branching channel.
 * line  - Line number in the application code (used for debugging).
 * fname - Filename name of the application code.
 *
 * Returns BC_SUCCESS if succes, else error code.
 */
int __bc_commit(bc_chan_t *bc, unsigned long line, const char *fname)
#else

int bc_commit(bc_chan_t *bc)
#endif
{
    if (!bc) {
        bc_perror(BC_EIBC, line, fname);
        return BC_EIBC;
    }
    if (bc->mode != __BC_PRODUCER) {
        bc_perror(BC_EIFUNC, line, fname);
        return BC_EIFUNC;
    }
    /*
     * Invoke the put() interface associated with the
     * branching channel role.
     */
    return bc_iface[bc->channel->role].p(bc->channel, NULL, 1);
}

#ifdef BC_SHOW_ERROR
/*
 * get data from the branching channel.
 *
 * bc    - Branching channel.
 * local - Local application buffer.
 * du    - Number of data units to get from branching channel.
 * line  - Line number in the application code (used for debugging).
 * fname - Filename name of the application code.
 *
 * Returns BC_SUCCESS if succes, else error code.
 */
int __bc_get(bc_chan_t *bc, void *local, int du, unsigned long line,
        const char *fname)
#else

int bc_get(bc_chan_t *bc, void *local, int du)
#endif
{
    if (!bc) {
        bc_perror(BC_EIBC, line, fname);
        return BC_EIBC;
    }
    if (bc->mode != __BC_CONSUMER) {
        bc_perror(BC_EIFUNC, line, fname);
        return BC_EIFUNC;
    }
    if (!local) {
        bc_perror(BC_EIPTR, line, fname);
        return BC_EIPTR;
    }
    if (du < 1) {
        bc_perror(BC_EIVAL, line, fname);
        return BC_EIVAL;
    }
    /*
     * Invoke the get() interface associated with the
     * branching channel role.
     */
    return bc_iface[bc->channel->role].r(bc, local, du);
}
