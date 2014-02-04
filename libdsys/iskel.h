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
#error "Please do not include 'iskel.h'; include 'dsys.h' instead."
#endif

#ifndef _BC_ISKEL_H
#define _BC_ISKEL_H

#include "config.h"
#include "base.h"
#include "branch.h"
#include "common.h"
#include "dtype.h"
#include "infmpi.h"
#include "ltab.h"
#include "iface.h"
#include "plist.h"
#include "service.h"
#include "sockets.h"
#include "threads.h"
#include "timing.h"

BEGIN_C_DECLS

/* Pipeline topology. */
typedef enum {
    __ISKEL_PIPE_FIRST = 0,
    __ISKEL_PIPE_LAST,
    __ISKEL_PIPE_INTER
} __iskel_pipe_role_t;

typedef struct iskel_pipe_dmap_s iskel_pipe_dmap_t;

struct iskel_pipe_dmap_s {
    bc_dtype_t *in;
    bc_dtype_t *out;
};
typedef void (*iskel_pipe_fptr_t)(void **v, bc_chan_t *i, bc_chan_t *o);

typedef struct iskel_pipe_s iskel_pipe_t;

struct iskel_pipe_s {
    int mc;
    __iskel_pipe_role_t role;
    iskel_pipe_fptr_t fptr;
    int pcount, ccount;
    bc_plist_t **prod;
    bc_plist_t **cons;
    bc_chan_t **src;
    bc_chan_t **sink;
};

extern iskel_pipe_t *
iskel_pipe_create(bc_plist_t *pl, iskel_pipe_fptr_t fptr[],
        iskel_pipe_dmap_t *dmap, int mc);

#define iskel_pipe_exec(pipe, in, out) \
        if ((pipe) != NULL) { \
            if ((pipe)->role == __ISKEL_PIPE_FIRST) { \
                (pipe)->fptr((void **)(in), NULL, (pipe)->sink[0]); \
            } else if ((pipe)->role == __ISKEL_PIPE_LAST) { \
                (pipe)->fptr((void **)(out), (pipe)->src[0], NULL); \
            } else { \
                (pipe)->fptr(NULL, (pipe)->src[0], (pipe)->sink[0]); \
            } \
        }

#define iskel_pipe_destroy(pipe) \
        if ((pipe) != NULL) { \
            if ((pipe)->role == __ISKEL_PIPE_FIRST) { \
                bc_chan_destroy((pipe)->sink[0]); \
                bc_plist_destroy((pipe)->cons[0]); \
                bc_free((pipe)->cons); \
                bc_free((pipe)->sink); \
            } else if ((pipe)->role == __ISKEL_PIPE_LAST) { \
                bc_chan_destroy((pipe)->src[0]); \
                bc_plist_destroy((pipe)->prod[0]); \
                bc_free((pipe)->prod); \
                bc_free((pipe)->src); \
            } else { \
                bc_chan_destroy((pipe)->src[0]); \
                bc_plist_destroy((pipe)->prod[0]); \
                bc_chan_destroy((pipe)->sink[0]); \
                bc_plist_destroy((pipe)->cons[0]); \
                bc_free((pipe)->prod); \
                bc_free((pipe)->cons); \
                bc_free((pipe)->src); \
                bc_free((pipe)->sink); \
            } \
            bc_free((pipe)); \
        }

/* All-to-all topology. */
typedef struct iskel_all2all_s iskel_all2all_t;

struct iskel_all2all_s {
    int num;
    int root;
    bc_plist_t *plist;
    bc_chan_t *src, *sink;
};

extern iskel_all2all_t *
iskel_all2all_create(bc_plist_t *pl, bc_dtype_t *dtype);

#define iskel_all2all_exec(topo, in, out, type)	\
        if ((topo) != NULL && (in) != NULL && (out) != NULL) { \
            if (bc_rank == (topo)->root) { \
                *(type *)(out) = *(type *)(in); \
                bc_get((topo)->src, (type *)(out) + 1, 1); \
                bc_put((topo)->sink, (out), (topo)->num); \
            } else { \
                bc_put((topo)->sink, (in), 1); \
                bc_get((topo)->src, (out), (topo)->num); \
            } \
        }
	
#define iskel_all2all_destroy(topo) \
        if ((topo) != NULL) { \
            bc_chan_destroy((topo)->src); \
            bc_chan_destroy((topo)->sink); \
            __bc_internal_plist_destroy((topo)->plist); \
            bc_free((topo)); \
        }														

/* Farm topology. */
typedef struct iskel_farm_dmap_s iskel_farm_dmap_t;

struct iskel_farm_dmap_s {
    bc_dtype_t *in;
    bc_dtype_t *out;
};

typedef enum {
    __ISKEL_FARM_FARMER = 0,
    __ISKEL_FARM_WORKER
} __iskel_farm_role_t;

typedef void (*iskel_farm_fptr_t)(void **vi, void **vo,
        bc_chan_t *i, bc_chan_t *o);

typedef struct iskel_farm_s iskel_farm_t;

struct iskel_farm_s {
    int mc;
    int num;
    __iskel_farm_role_t role;
    iskel_farm_fptr_t fptr;
    bc_plist_t *plist;
    bc_chan_t *src, *sink;
};

extern iskel_farm_t *
iskel_farm_create(bc_plist_t *pl, iskel_farm_fptr_t farmer,
        iskel_farm_fptr_t worker, iskel_farm_dmap_t *dmap, int mc);

#define iskel_farm_exec(farm, in, out) \
        if ((farm) != NULL && (in) != NULL && (out) != NULL) { \
            if ((farm)->role == __ISKEL_FARM_FARMER) \
                (farm)->fptr((void **)(in), (void **)(out), \
                (farm)->src, (farm)->sink); \
            else \
                (farm)->fptr(NULL, NULL, (farm)->src, (farm)->sink); \
        }
	
#define iskel_farm_destroy(farm) \
        if ((farm) != NULL) { \
            bc_chan_destroy((farm)->src); \
            bc_chan_destroy((farm)->sink); \
            __bc_internal_plist_destroy((farm)->plist); \
            bc_free((farm)); \
        }

END_C_DECLS

#endif /* BC_ISKEL_H */
