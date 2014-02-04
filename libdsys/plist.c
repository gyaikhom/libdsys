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

/* Description:
 * Process lists are relevant only to the computation thread. Hence,
 * it can be destroyed by the computation thread when not needed. There
 * are two ways in which a process list can be destroyed.
 *
 * (1) bc_despl() - This is direct request for destroying the process list.
 * (2) from bc_desbc() - When the branching channel does not require the
 *     process list anymore.
 *
 * When a process list is created, it has a reference count ('rc') of 1.
 * This reference is made by the bc_plist_t declared by the computation
 * thread. When a branching channel is created, the reference count is
 * incremented. Hence, we decrement the reference count for any of the two
 * destroy process list invocations. If the reference count is 0, we can
 * actually destroy and free the resources.
 */

#include "base.h"
#include "mem.h"
#include "plist.h"
#include "threads.h"
#include <stdarg.h>

bc_plist_t *__bc_optional_plists[__BC_NUM_OPTIONAL_PLISTS] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static int __bc_plist_equal_division(bc_plist_t *pl, bc_plist_t *plptr[],
        int num);
static int compare(const void *, const void *);

int compare(const void *a, const void *b) {
    return *(int *) a - *(int *) b;
}

int bc_plist_iselem(bc_plist_t *pl, int rank) {
    int i;

    if (!pl)
        return 0;

    for (i = 0; i < pl->count; i++)
        if (pl->plist[i] == bc_rank)
            return 1;
    return 0;
}

int bc_plist_nelem(bc_plist_t *pl) {
    if (!pl)
        return 0;
    return pl->count;
}

int bc_plist_isvalid(bc_plist_t *pl) {
    if (!pl)
        return 0;
    return !(pl->valid);
}

bc_plist_t *bc_plist_create(int count, ...) {
    register int i;
    bc_plist_t *pl;
    va_list aptr;

    if (!count)
        return NULL;

    if (!(pl = bc_malloc(bc_plist_t, 1)))
        return NULL;

    if (count > 0) {
        if (!(pl->plist = bc_malloc(int, count))) {
            bc_free(pl);
            return NULL;
        }
        va_start(aptr, count);
        for (i = 0; i < count; i++)
            pl->plist[i] = va_arg(aptr, int);
        va_end(aptr);
        pl->count = count;
    } else {
        register int j, s, e;
        int *args;

        count = -count;
        pl->count = bc_base_layer->mpi->size - count;

        if (!(pl->plist = bc_malloc(int, pl->count))) {
            bc_free(pl);
            return NULL;
        }
        if (!(args = bc_malloc(int, count))) {
            bc_free(pl->plist);
            bc_free(pl);
            return NULL;
        }
        va_start(aptr, count);
        for (i = 0; i < count; i++)
            args[i] = va_arg(aptr, int);
        va_end(aptr);

        /* Sort the provided processes. */
        qsort(args, count, sizeof (int), compare);

        /* Enter value between any consecutive pair. */
        s = i = j = 0;
        while (i < count) {
            e = args[i++];
            while (s < e)
                pl->plist[j++] = s++;
            s++;
        }
        while (s < bc_base_layer->mpi->size)
            pl->plist[j++] = s++;
        bc_free(args);
    }

    /*
     * We count existence of the plist itself as a reference.
     * This is required because the plist can still exists after a
     * bc_despl() has been invoked by the producer if consumers 
     * are still depending on the branching channel(BC), which 
     * requires the plist for consistency. Also, the plist can be
     * destroyed with explicit call to bc_despl(). Hence, we have
     * to set rc to 1, to account for the bc_despl().
     */
    pl->rc = 1;
    pl->type = __BC_PLIST_CUST;
    bc_mutex_init(&(pl->lock));
    return pl;
}

#ifdef BC_SHOW_ERROR
int __bc_plist_destroy(bc_plist_t *pl, unsigned long line, const char *fname)
#else

int bc_plist_destroy(bc_plist_t *pl)
#endif
{
    if (!pl) {
        bc_perror(BC_EIPLIST, line, fname);
        return BC_EIPLIST;
    }
    return __bc_internal_plist_destroy(pl);
}

int __bc_internal_plist_destroy(bc_plist_t *pl) {
    if (pl->type != __BC_PLIST_CUST)
        return BC_SUCCESS;

    bc_mutex_lock(&(pl->lock));
    pl->rc--;
    if (pl->rc)
        bc_mutex_unlock(&(pl->lock));
    else {
        bc_free(pl->plist);
        bc_mutex_unlock(&(pl->lock));
        bc_mutex_destroy(&(pl->lock));
        bc_free(pl);
    }
    return BC_SUCCESS;
}

#ifdef BC_SHOW_ERROR
int __bc_plist_set(bc_plist_t *pl, int loc, int value, unsigned long line,
        const char *fname)
#else

int bc_plist_set(bc_plist_t *pl, int loc, int value)
#endif
{
    /* Invalid plist/Invalid location: out of bound. */
    if (!pl) {
        bc_perror(BC_EIPLIST, line, fname);
        return BC_EIPLIST;
    }

    if (loc > pl->count) {
        bc_perror(BC_EIVAL, line, fname);
        return BC_EIVAL;
    }

    /* New value was set; plist valid only when 'valid' == 0. */
    if (pl->plist[loc] == -1) { /* If '-1', it is not reset. */
        pl->valid--;
        pl->plist[loc] = value;
        return (pl->valid); /* Number of processes required. */
    } else {
        /* Reset request: Lock to check reference count. */
        bc_mutex_lock(&(pl->lock));
        if (pl->rc > 1) {
            bc_mutex_unlock(&(pl->lock));
            return BC_SUCCESS; /* Cannot reset plist: In use. */
        }
        pl->plist[loc] = value;
        bc_mutex_unlock(&(pl->lock));
        return BC_SUCCESS;
    }
}

#ifdef BC_SHOW_ERROR
bc_plist_t *__bc_plist_create_empty(int count, unsigned long line,
        const char *fname)
#else

bc_plist_t *bc_plist_create_empty(int count)
#endif
{
    bc_plist_t *pl;
    register int i;

    if (count <= 0) {
        bc_perror(BC_EIVAL, line, fname);
        return NULL;
    }

    if (!(pl = bc_malloc(bc_plist_t, 1))) {
        bc_perror(BC_EMEM, line, fname);
        return NULL;
    }

    if (!(pl->plist = bc_malloc(int, count))) {
        bc_free(pl);
        bc_perror(BC_EMEM, line, fname);
        return NULL;
    }

    pl->type = __BC_PLIST_CUST;
    pl->count = count;
    pl->valid = count;
    for (i = 0; i < count; i++)
        pl->plist[i] = -1;
    pl->rc = 1;
    bc_mutex_init(&(pl->lock));
    return pl;
}

#ifdef BC_SHOW_ERROR
int __bc_plist_display(bc_plist_t *pl, unsigned long line, const char *fname)
#else

int bc_plist_display(bc_plist_t *pl)
#endif
{
    int i, temp;

    if (!pl) {
        bc_perror(BC_EIPLIST, line, fname);
        return BC_EIPLIST;
    }

    if (pl->valid)
        printf("[%d] plist (INVALID): {", bc_rank);
    else
        printf("[%d] plist: {", bc_rank);

    temp = pl->count - 1;
    for (i = 0; i < temp; i++)
        printf("%d, ", pl->plist[i]);
    printf("%d}\n", pl->plist[temp]);

    return 0;
}

int bc_plist_divide(bc_plist_t *pl, bc_plist_t *plptr[],
        bc_divpl_t type, ...) {
    int ret_val;
    va_list aptr;

    if (!pl || !plptr)
        return -1;

    va_start(aptr, type);
    switch (type) {
        case BC_DIVPL_EQUAL:
            ret_val = __bc_plist_equal_division(pl, plptr, va_arg(aptr, int));
            va_end(aptr);
            return ret_val;
        default:
            break;
    }
    return 0;
}

int __bc_plist_equal_division(bc_plist_t *pl, bc_plist_t *plptr[], int num) {
    int i, j, k, cursor = 0, extra, per_plist;

    /* Not enough processes to satisfy the division. */
    if (num > pl->count)
        return -1;

    /* Create new plists. */
    per_plist = pl->count / num;
    if ((extra = pl->count % num))
        k = num - 1;
    else
        k = num;

    for (i = 0; i < k; i++) {
        plptr[i] = bc_plist_create_empty(per_plist);
        for (j = 0; j < per_plist; j++)
            bc_plist_set(plptr[i], j, pl->plist[cursor++]);
    }
    if (extra) {
        per_plist += extra;
        plptr[k] = bc_plist_create_empty(per_plist);
        for (j = 0; j < per_plist; j++)
            bc_plist_set(plptr[k], j, pl->plist[cursor++]);
    }
    return 0;
}

bc_plist_t *bc_plist_union(int num, ...) {
    bc_plist_t **plptr;
    bc_plist_t *pl;
    va_list aptr;
    int total = 0;
    int *list, i, j, k, cursor;

    if (!(plptr = bc_malloc(bc_plist_t *, num)))
        return NULL;

    va_start(aptr, num);
    for (i = 0; i < num; i++) {
        plptr[i] = va_arg(aptr, bc_plist_t *);
    }
    va_end(aptr);

    for (i = 0; i < num; i++) {
        /* Some of the plists are invalid. */
        if (plptr[i]->valid) {
            bc_free(plptr);
            return NULL;
        }
        total += plptr[i]->count;
    }

    if (!(list = bc_malloc(int, total))) {
        bc_free(plptr);
        return NULL;
    }

    /* Copy the first plist. */
    for (i = 0; i < plptr[0]->count; i++)
        list[i] = plptr[0]->plist[i];
    cursor = i;

    /* Do the union. */
    for (i = 1; i < num; i++) {
        for (j = 0; j < plptr[i]->count; j++) {
            k = 0;
            while (1) {
                if (k < cursor) {
                    /* Value already in list. */
                    if (list[k] == plptr[i]->plist[j])
                        break;
                    k++;
                } else {
                    list[k] = plptr[i]->plist[j];
                    cursor++;
                    break;
                }
            }
        }
    }

    if (!(pl = bc_malloc(bc_plist_t, 1))) {
        bc_free(plptr);
        return NULL;
    }

    bc_free(plptr);
    pl->plist = list;
    pl->type = __BC_PLIST_CUST;
    pl->rc = 1;
    pl->valid = 0;
    pl->count = cursor;
    pl->plist = bc_realloc(pl->plist, int, pl->count);
    bc_mutex_init(&(pl->lock));
    return pl;
}

bc_plist_t *bc_plist_isect(int num, ...) {
    bc_plist_t **plptr;
    bc_plist_t *pl;
    va_list aptr;
    int total = 0;
    int *list, i, j, k, cursor;
    int *inclusion;

    if (!(plptr = bc_malloc(bc_plist_t *, num)))
        return NULL;

    va_start(aptr, num);
    for (i = 0; i < num; i++) {
        plptr[i] = va_arg(aptr, bc_plist_t *);
    }
    va_end(aptr);

    for (i = 0; i < num; i++) {
        /* Some of the plists are invalid. */
        if (plptr[i]->valid) {
            bc_free(plptr);
            return NULL;
        }
        total += plptr[i]->count;
    }

    if (!(list = bc_malloc(int, total))) {
        bc_free(plptr);
        return NULL;
    }

    if (!(inclusion = bc_malloc(int, total))) {
        bc_free(list);
        bc_free(plptr);
        return NULL;
    }

    /* Copy the first plist. */
    for (i = 0; i < plptr[0]->count; i++) {
        list[i] = plptr[0]->plist[i];
        inclusion[i] = 1;
    }
    cursor = i;

    /* Do the intersection, and count. */
    for (i = 1; i < num; i++) {
        for (j = 0; j < plptr[i]->count; j++) {
            k = 0;
            while (1) {
                if (k < cursor) {
                    /* Value already in list. */
                    if (list[k] == plptr[i]->plist[j]) {
                        inclusion[k]++;
                        break;
                    }
                    k++;
                } else {
                    list[k] = plptr[i]->plist[j];
                    inclusion[k] = 1;
                    cursor++;
                    break;
                }
            }
        }
    }

    bc_free(plptr);

    /* Calculate the intersection. */
    k = 0;
    for (i = 0; i < cursor; i++) {
        if (inclusion[i] == num)
            list[k++] = list[i];
    }
    bc_free(inclusion);

    if (k == 0)
        return NULL;

    if (!(pl = bc_malloc(bc_plist_t, 1))) {
        bc_free(plptr);
        return NULL;
    }

    pl->plist = list;
    pl->type = __BC_PLIST_CUST;
    pl->rc = 1;
    pl->valid = 0;
    pl->count = k;
    pl->plist = bc_realloc(pl->plist, int, pl->count);
    bc_mutex_init(&(pl->lock));
    return pl;
}

bc_plist_t *bc_plist_diff(bc_plist_t *a, bc_plist_t *b) {
    bc_plist_t *pl;
    register int i, j;

    if (!a || !b)
        return NULL;

    /* A Process list is valid only when 'valid' == 0. */
    if (a->valid || b->valid)
        return NULL;

    if (!(pl = bc_malloc(bc_plist_t, 1)))
        return NULL;

    if (!(pl->plist = bc_malloc(int, a->count))) {
        bc_free(pl);
        return NULL;
    }

    /* Choose none. */
    for (i = 0; i < a->count; i++)
        pl->plist[i] = 0;

    /* For each process in the second set. */
    for (i = 0; i < b->count; i++) {
        for (j = 0; j < a->count; j++) {
            /* Flag. */
            if (a->plist[j] == b->plist[i]) {
                pl->plist[j] = 1;
                break;
            }
        }
    }

    /* Choose not flagged. */
    for (i = 0, j = 0; i < a->count; i++) {
        if (!pl->plist[i])
            pl->plist[j++] = a->plist[i];
    }

    pl->type = __BC_PLIST_CUST;
    pl->rc = 1;
    pl->valid = 0;
    pl->count = j;
    pl->plist = bc_realloc(pl->plist, int, j);
    bc_mutex_init(&(pl->lock));
    return pl;
}

/*
 * Process lists exclude the calling process.
 */
int __bc_optional_plists_create(void) {
    register int i, j;

    /* Process lists with all the processes excluding self. */
    if (__bc_internal_flags & BC_PLIST_XALL) {
        if (!(bc_plist_xall = bc_malloc(bc_plist_t, 1)))
            goto error;
        if (!(bc_plist_xall->plist = bc_malloc(int, bc_size - 1))) {
            bc_free(bc_plist_xall);
            goto error;
        }
        for (i = 0, j = 0; i < bc_size; i++)
            if (i != bc_rank)
                bc_plist_xall->plist[j++] = i;
        bc_plist_xall->valid = 0;
        bc_plist_xall->count = j;
        bc_plist_xall->type = __BC_PLIST_INTERNAL;
    }

    /* Process lists with all odd processes. */
    if (__bc_internal_flags & BC_PLIST_ODD) {
        if (!(bc_plist_odd = bc_malloc(bc_plist_t, 1)))
            goto error;
        if (!(bc_plist_odd->plist = bc_malloc(int, bc_size - 1))) {
            bc_free(bc_plist_odd);
            goto error;
        }
        for (i = 1, j = 0; i < bc_size; i += 2)
            if (i != bc_rank)
                bc_plist_odd->plist[j++] = i;
        bc_plist_odd->valid = 0;
        bc_plist_odd->count = j;
        bc_plist_odd->plist = bc_realloc(bc_plist_odd->plist, int, j);
        bc_plist_odd->type = __BC_PLIST_INTERNAL;
    }

    /* Process lists with all even processes. */
    if (__bc_internal_flags & BC_PLIST_EVEN) {
        if (!(bc_plist_even = bc_malloc(bc_plist_t, 1)))
            goto error;
        if (!(bc_plist_even->plist = bc_malloc(int, bc_size - 1))) {
            bc_free(bc_plist_even);
            goto error;
        }
        for (i = 0, j = 0; i < bc_size; i += 2)
            if (i != bc_rank)
                bc_plist_even->plist[j++] = i;
        bc_plist_even->valid = 0;
        bc_plist_even->count = j;
        bc_plist_even->plist = bc_realloc(bc_plist_even->plist, int, j);
        bc_plist_even->type = __BC_PLIST_INTERNAL;
    }

    /* Process lists with preceding processes. */
    if (__bc_internal_flags & BC_PLIST_PRED) {
        if (bc_rank != 0) {
            if (!(bc_plist_pred = bc_malloc(bc_plist_t, 1)))
                goto error;
            if (!(bc_plist_pred->plist = bc_malloc(int, bc_rank))) {
                bc_free(bc_plist_pred);
                goto error;
            }
            for (i = 0; i < bc_rank; i++)
                bc_plist_pred->plist[i] = i;
            bc_plist_pred->valid = 0;
            bc_plist_pred->count = i;
            bc_plist_pred->type = __BC_PLIST_INTERNAL;
        }
    }

    /* Process lists with succeeding processes. */
    if (__bc_internal_flags & BC_PLIST_SUCC) {
        i = bc_size - 1;
        if (bc_rank != i) {
            if (!(bc_plist_succ = bc_malloc(bc_plist_t, 1)))
                goto error;
            if (!(bc_plist_succ->plist = bc_malloc(int, i - bc_rank))) {
                bc_free(bc_plist_succ);
                goto error;
            }
            for (i = bc_rank + 1, j = 0; i < bc_size; i++)
                bc_plist_succ->plist[j++] = i;
            bc_plist_succ->valid = 0;
            bc_plist_succ->count = j;
            bc_plist_succ->type = __BC_PLIST_INTERNAL;
        }
    }

    /* Process lists with all the processes. */
    if (__bc_internal_flags & BC_PLIST_ALL) {
        if (!(bc_plist_all = bc_malloc(bc_plist_t, 1)))
            goto error;
        if (!(bc_plist_all->plist = bc_malloc(int, bc_size))) {
            bc_free(bc_plist_all);
            goto error;
        }
        for (i = 0; i < bc_size; i++)
            bc_plist_all->plist[i] = i;
        bc_plist_all->valid = 0;
        bc_plist_all->count = i;
        bc_plist_all->type = __BC_PLIST_INTERNAL;
    }

    /* Process lists with self. */
    if (__bc_internal_flags & BC_PLIST_SELF) {
        if (!(bc_plist_self = bc_malloc(bc_plist_t, 1)))
            goto error;
        if (!(bc_plist_self->plist = bc_malloc(int, 1))) {
            bc_free(bc_plist_self);
            goto error;
        }
        bc_plist_self->plist[0] = bc_rank;
        bc_plist_self->valid = 0;
        bc_plist_self->count = 1;
        bc_plist_self->type = __BC_PLIST_INTERNAL;
    }
    return BC_SUCCESS;

error:
    __bc_optional_plists_destroy();
    return BC_EMEM;
}

void __bc_optional_plists_destroy(void) {
    int i;

    for (i = 0; i < __BC_NUM_OPTIONAL_PLISTS; i++)
        if (__bc_optional_plists[i]) {
            bc_free(__bc_optional_plists[i]->plist);
            bc_free(__bc_optional_plists[i]);
        }
}
