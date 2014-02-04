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
#error "Please do not include 'plist.h'; include 'dsys.h' instead."
#endif

#ifndef _BC_PLIST_H
#define _BC_PLIST_H

#include "common.h"
#include "threads.h"

BEGIN_C_DECLS

enum {
    __BC_PLIST_CUST = 0, __BC_PLIST_INTERNAL
};

typedef enum {
    BC_DIVPL_EQUAL = 0,

    BC_DIVPL_NUM
} bc_divpl_t;

typedef struct bc_plist_s {
    int type; /* Custom or default. */
    int count; /* Number of processes. */
    int valid; /* Validity of the plist. */
    int *plist; /* Process list. */
    bc_mutex_t lock; /* Lock for modifying rc. */
    int rc; /* Reference count. */
} bc_plist_t;

/* Create plist with the listed values. */
extern bc_plist_t *bc_plist_create(int count, ...);

#ifdef BC_SHOW_ERROR

/* Create an empty plist. Used in conjunction with bc_setpl(). */
extern bc_plist_t *__bc_plist_create_empty(int count, unsigned long line,
        const char *fname);

/* Destroy a plist. */
extern int __bc_plist_destroy(bc_plist_t *plist, unsigned long line,
        const char *fname);

/* Sets a new value/resets a value if value already set at location. */
extern int __bc_plist_set(bc_plist_t *plist, int loc, int value,
        unsigned long line, const char *fname);

/* Show plist information. */
extern int __bc_plist_display(bc_plist_t *pl, unsigned long line,
        const char *fname);

#define bc_plist_set(plist, loc, value) \
        __bc_plist_set((plist), (loc), (value), __LINE__, __FILE__)

#define	bc_plist_create_empty(count) \
        __bc_plist_create_empty((count), __LINE__, __FILE__)

#define bc_plist_display(plist) \
        __bc_plist_display((plist), __LINE__, __FILE__)

#define bc_plist_destroy(plist) \
        __bc_plist_destroy((plist), __LINE__, __FILE__)

#else

/* Create an empty plist. Used in conjunction with bc_setpl(). */
extern bc_plist_t *bc_plist_create_empty(int count);

/* Destroy a plist. */
extern int bc_plist_destroy(bc_plist_t *plist);

/* Sets a new value/resets a value if value already set at location. */
extern int bc_plist_set(bc_plist_t *plist, int loc, int value);

/* Show plist information. */
extern int bc_plist_display(bc_plist_t *pl);

#endif

/* Divide plist. */
extern int bc_plist_divide(bc_plist_t *pl, bc_plist_t *plptr[],
        bc_divpl_t type, ...);

/* Set union. */
extern bc_plist_t *bc_plist_union(int num, ...);

/* Set intersection. */
extern bc_plist_t *bc_plist_isect(int num, ...);

/* Set difference. */
extern bc_plist_t *bc_plist_diff(bc_plist_t *a, bc_plist_t *b);

/* Create optional process lists. */
extern int __bc_optional_plists_create(void);

/* Destroy optional process lists. */
extern void __bc_optional_plists_destroy(void);

/* Is an element . */
extern int bc_plist_iselem(bc_plist_t *pl, int rank);

/* Number of elements. */
extern int bc_plist_nelem(bc_plist_t *pl);

/* Is the plist valid. */
extern int bc_plist_isvalid(bc_plist_t *pl);

extern int __bc_internal_plist_destroy(bc_plist_t *pl);

#define __bc_internal_plist_ref(plist) \
        if ((plist)->type == __BC_PLIST_CUST) { \
            bc_mutex_lock(&((plist)->lock)); \
            (plist)->rc++; \
            bc_mutex_unlock(&((plist)->lock)); \
        }

#define __BC_NUM_OPTIONAL_PLISTS 7
#define BC_PLIST_XALL  0x00000002 /* Process lists with all other processes. */
#define BC_PLIST_ODD   0x00000004 /* Process lists with odd processes. */
#define BC_PLIST_EVEN  0x00000008 /* Process lists with even processes. */
#define BC_PLIST_PRED  0x00000010 /* Process lists with preceding processes. */
#define BC_PLIST_SUCC  0x00000020 /* Process lists with succeeding processes. */
#define BC_PLIST_ALL   0x00000040 /* Process lists with all processes. */
#define BC_PLIST_SELF  0x00000080 /* Process lists with self process. */

#define bc_plist_xall  __bc_optional_plists[0]
#define bc_plist_odd   __bc_optional_plists[1]
#define bc_plist_even  __bc_optional_plists[2]
#define bc_plist_pred  __bc_optional_plists[3]
#define bc_plist_succ  __bc_optional_plists[4]
#define bc_plist_all   __bc_optional_plists[5]
#define bc_plist_self  __bc_optional_plists[6]

extern bc_plist_t *__bc_optional_plists[];

END_C_DECLS

#endif /* _BC_PLIST_H */
