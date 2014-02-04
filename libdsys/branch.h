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
#error "Please do not include 'branch.h'; include 'dsys.h' instead."
#endif

#ifndef _BC_BRANCH_H
#define _BC_BRANCH_H

#include "common.h"
#include "dtype.h"
#include "plist.h"
#include "queue.h"

BEGIN_C_DECLS

enum {
    __BC_PRODUCER = 0,
    __BC_CONSUMER
};

typedef enum {
    BC_ROLE_CUST = 0,
    BC_ROLE_PIPE,
    BC_ROLE_PIPEN,
    BC_ROLE_REPLICATE,
    BC_ROLE_REPLICATEN,
    BC_ROLE_REDUCE_SUM,
    BC_ROLE_REDUCE_MUL,
    BC_ROLE_REDUCE_MAX,
    BC_ROLE_REDUCE_MIN,
    BC_ROLE_REDUCE_MINMAX,
    BC_ROLE_REDUCE_OPT,
    BC_ROLE_FARM,
    BC_ROLE_FARMN,
    BC_ROLE_COLLECT,
    BC_ROLE_SPREAD,
    BC_ROLE_COLLECT_ANY,

    BC_ROLE_NUM
} bc_role_t;

/*
 * A channel is shared by the branching channel and the link in the
 * lookup table.
 */
typedef struct {
    bc_role_t role;
    bc_qtype_t qtype;
    bc_dtype_t *dtype;
    bc_queue_t *q;
    unsigned int rc;
} bc_channel_t;

typedef struct {
    unsigned short mode;
    bc_plist_t *plist;
    unsigned long *tags;
    bc_channel_t *channel;
} bc_chan_t;

#ifdef BC_SHOW_ERROR /* Display error messages. */

extern bc_chan_t *__bc_chan_create(bc_plist_t *prod, bc_plist_t *cons,
        bc_dtype_t *dtype, unsigned int dunits,
        bc_role_t role, unsigned long line, const char *fname);

extern bc_chan_t *__bc_sink_create(bc_plist_t *cons, bc_dtype_t *dtype,
        unsigned int dunits, bc_role_t role,
        unsigned long lnum, const char *fname);

extern bc_chan_t *__bc_src_create(bc_plist_t *prod, bc_dtype_t *dtype,
        bc_role_t role, unsigned long line, const char *fname);

extern int __bc_chan_info(bc_chan_t *bc, unsigned long line,
        const char *fname);
extern int __bc_chan_destroy(bc_chan_t *bc, unsigned long line,
        const char *fname);
extern int __bc_put(bc_chan_t *bc, void *local, int du, unsigned long line,
        const char *fname);
extern int __bc_commit(bc_chan_t *bc, unsigned long line, const char *fname);
extern int __bc_get(bc_chan_t *bc, void *local, int du, unsigned long line,
        const char *fname);

#define bc_chan_create(prod, cons, dtype, duints, role) \
	__bc_chan_create((prod), (cons), (dtype), (duints), (role), \
            __LINE__, __FILE__);
#define bc_sink_create(cons, dtype, duints, role) \
	__bc_sink_create((cons), (dtype), (duints), (role), \
            __LINE__, __FILE__);
#define bc_src_create(cons, dtype, role) \
	__bc_src_create((cons), (dtype), (role), __LINE__, __FILE__);
#define bc_chan_info(bc) \
	__bc_chan_info((bc), __LINE__, __FILE__);
#define bc_chan_destroy(bc) \
	__bc_chan_destroy((bc), __LINE__, __FILE__);
#define bc_put(bc, local, du) \
	__bc_put((bc), (local), (du), __LINE__, __FILE__);
#define bc_get(bc, local, du) \
	__bc_get((bc), (local), (du), __LINE__, __FILE__);
#define bc_commit(bc) \
	__bc_commit((bc), __LINE__, __FILE__);

#else /* Display error messages. */

extern bc_chan_t *bc_chan_create(bc_plist_t *prod, bc_plist_t *cons,
        bc_dtype_t *dtype, unsigned int dunits,
        bc_role_t role);

extern bc_chan_t *bc_sink_create(bc_plist_t *cons, bc_dtype_t *dtype,
        unsigned int dunits, bc_role_t role);

extern bc_chan_t *bc_src_create(bc_plist_t *prod, bc_dtype_t *dtype, bc_role_t role);

extern int bc_chan_info(bc_chan_t *bc);
extern int bc_chan_destroy(bc_chan_t *bc);
extern int bc_put(bc_chan_t *bc, void *local, int du);
extern int bc_commit(bc_chan_t *bc);
extern int bc_get(bc_chan_t *bc, void *local, int du);

#endif /* Display error messages. */

END_C_DECLS

#endif /* _BC_BRANCH_H */
