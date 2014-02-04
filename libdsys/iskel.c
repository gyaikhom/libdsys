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

#include "iskel.h"

/**
 * PIPELINE:
 *
 * There are two types of the pipeline implementation.
 * (1) With memory copy.
 * (2) Without memory copy.
 *
 * The second implementation is more likely to be used for a pipeline.
 */
iskel_pipe_t *
iskel_pipe_create(bc_plist_t *pl, iskel_pipe_fptr_t fptr[],
        iskel_pipe_dmap_t *dmap, int mc) {
    int i = 0;
    int last;
    iskel_pipe_t *pipe;

    if (!bc_plist_isvalid(pl) ||
            !bc_plist_iselem(pl, bc_rank) ||
            !fptr || !dmap)
        return NULL;

    /* Pipeline should have atleast two stages. */
    if (pl->count < 2)
        return NULL;
    last = pl->count - 1;

    if (!(pipe = bc_malloc(iskel_pipe_t, 1)))
        return NULL;

    /* With memory copy. */
    if (mc) {
        do {
            if (bc_rank == pl->plist[i]) {
                if (i == 0) {
                    /* Create consumer branching channel. */
                    if (!(pipe->cons = bc_malloc(bc_plist_t *, 1))) {
                        bc_free(pipe);
                        return NULL;
                    }
                    if (!(pipe->sink = bc_malloc(bc_chan_t *, 1))) {
                        bc_free(pipe->cons);
                        bc_free(pipe);
                        return NULL;
                    }
                    pipe->cons[0] = bc_plist_create(1, pl->plist[i + 1]);
                    pipe->sink[0] = bc_sink_create(pipe->cons[0], dmap[i].out,
                            7, BC_ROLE_PIPE);
                    pipe->role = __ISKEL_PIPE_FIRST;
                    pipe->fptr = fptr[i];

                } else if (i == last) {
                    /* Create producer branching channel. */
                    if (!(pipe->prod = bc_malloc(bc_plist_t *, 1))) {
                        bc_free(pipe);
                        return NULL;
                    }
                    if (!(pipe->src = bc_malloc(bc_chan_t *, 1))) {
                        bc_free(pipe->prod);
                        bc_free(pipe);
                        return NULL;
                    }

                    pipe->prod[0] = bc_plist_create(1, pl->plist[i - 1]);
                    pipe->src[0] = bc_src_create(pipe->prod[0], dmap[i].in,
                            BC_ROLE_PIPE);
                    pipe->role = __ISKEL_PIPE_LAST;
                    pipe->fptr = fptr[i];
                } else {
                    if (!(pipe->prod = bc_malloc(bc_plist_t *, 1))) {
                        bc_free(pipe);
                        return NULL;
                    }
                    if (!(pipe->src = bc_malloc(bc_chan_t *, 1))) {
                        bc_free(pipe->prod);
                        bc_free(pipe);
                        return NULL;
                    }
                    if (!(pipe->cons = bc_malloc(bc_plist_t *, 1))) {
                        bc_free(pipe->prod);
                        bc_free(pipe->src);
                        bc_free(pipe);
                        return NULL;
                    }
                    if (!(pipe->sink = bc_malloc(bc_chan_t *, 1))) {
                        bc_free(pipe->prod);
                        bc_free(pipe->src);
                        bc_free(pipe->cons);
                        bc_free(pipe);
                        return NULL;
                    }

                    pipe->prod[0] = bc_plist_create(1, pl->plist[i - 1]);
                    pipe->src[0] = bc_src_create(pipe->prod[0], dmap[i].in,
                            BC_ROLE_PIPE);

                    pipe->cons[0] = bc_plist_create(1, pl->plist[i + 1]);
                    pipe->sink[0] = bc_sink_create(pipe->cons[0], dmap[i].out,
                            7, BC_ROLE_PIPE);
                    pipe->role = __ISKEL_PIPE_INTER;
                    pipe->fptr = fptr[i];
                }
                break;
            }
            i++;
            if (i == pl->count) {
                bc_free(pipe);
                return NULL;
            }
        } while (1);
    } else {
        do {
            if (bc_rank == pl->plist[i]) {
                if (i == 0) {
                    /* Create consumer branching channel. */
                    if (!(pipe->cons = bc_malloc(bc_plist_t *, 1))) {
                        bc_free(pipe);
                        return NULL;
                    }
                    if (!(pipe->sink = bc_malloc(bc_chan_t *, 1))) {
                        bc_free(pipe->cons);
                        bc_free(pipe);
                        return NULL;
                    }
                    pipe->cons[0] = bc_plist_create(1, pl->plist[i + 1]);
                    pipe->sink[0] = bc_sink_create(pipe->cons[0], dmap[i].out,
                            10, BC_ROLE_PIPEN);
                    pipe->role = __ISKEL_PIPE_FIRST;
                    pipe->fptr = fptr[i];

                } else if (i == last) {
                    /* Create producer branching channel. */
                    if (!(pipe->prod = bc_malloc(bc_plist_t *, 1))) {
                        bc_free(pipe);
                        return NULL;
                    }
                    if (!(pipe->src = bc_malloc(bc_chan_t *, 1))) {
                        bc_free(pipe->prod);
                        bc_free(pipe);
                        return NULL;
                    }

                    pipe->prod[0] = bc_plist_create(1, pl->plist[i - 1]);
                    pipe->src[0] = bc_src_create(pipe->prod[0], dmap[i].in,
                            BC_ROLE_PIPEN);
                    pipe->role = __ISKEL_PIPE_LAST;
                    pipe->fptr = fptr[i];
                } else {
                    if (!(pipe->prod = bc_malloc(bc_plist_t *, 1))) {
                        bc_free(pipe);
                        return NULL;
                    }
                    if (!(pipe->src = bc_malloc(bc_chan_t *, 1))) {
                        bc_free(pipe->prod);
                        bc_free(pipe);
                        return NULL;
                    }
                    if (!(pipe->cons = bc_malloc(bc_plist_t *, 1))) {
                        bc_free(pipe->prod);
                        bc_free(pipe->src);
                        bc_free(pipe);
                        return NULL;
                    }
                    if (!(pipe->sink = bc_malloc(bc_chan_t *, 1))) {
                        bc_free(pipe->prod);
                        bc_free(pipe->src);
                        bc_free(pipe->cons);
                        bc_free(pipe);
                        return NULL;
                    }

                    pipe->prod[0] = bc_plist_create(1, pl->plist[i - 1]);
                    pipe->src[0] = bc_src_create(pipe->prod[0], dmap[i].in,
                            BC_ROLE_PIPEN);

                    pipe->cons[0] = bc_plist_create(1, pl->plist[i + 1]);
                    pipe->sink[0] = bc_sink_create(pipe->cons[0], dmap[i].out,
                            10, BC_ROLE_PIPEN);
                    pipe->role = __ISKEL_PIPE_INTER;
                    pipe->fptr = fptr[i];
                }
                break;
            }
            i++;
            if (i == pl->count) {
                bc_free(pipe);
                return NULL;
            }
        } while (1);
    }
    pipe->mc = mc;
    return pipe;
}

iskel_all2all_t *iskel_all2all_create(bc_plist_t *pl, bc_dtype_t *dtype) {
    iskel_all2all_t *topo;
    register int i;

    /* Check if I should participate. */
    if (!bc_plist_isvalid(pl) || !bc_plist_iselem(pl, bc_rank))
        return NULL; /* I should not participate. */

    if (!(topo = bc_malloc(iskel_all2all_t, 1)))
        return NULL;

    /* Find the root (the smallest rank). */
    topo->root = pl->plist[0];
    for (i = 1; i < pl->count; i++)
        if (pl->plist[i] < topo->root)
            topo->root = pl->plist[i];

    topo->num = pl->count;

    if (bc_rank == topo->root) {
        if (!bc_plist_self) {
            if (!(bc_plist_self = bc_malloc(bc_plist_t, 1))) {
                bc_free(topo);
                return NULL;
            }
            if (!(bc_plist_self->plist = bc_malloc(int, 1))) {
                bc_free(bc_plist_self);
                bc_free(topo);
                return NULL;
            }
            bc_plist_self->plist[0] = bc_rank;
            bc_plist_self->valid = 0;
            bc_plist_self->count = 1;
            bc_plist_self->type = __BC_PLIST_INTERNAL;
            __bc_internal_flags |= BC_PLIST_SELF;
        }
        topo->plist = bc_plist_diff(pl, bc_plist_self);
        topo->src = bc_src_create(topo->plist, dtype, BC_ROLE_COLLECT);
        topo->sink = bc_sink_create(topo->plist, dtype, topo->num,
                BC_ROLE_REPLICATE);
    } else {
        topo->plist = bc_plist_create(1, topo->root);
        topo->src = bc_src_create(topo->plist, dtype, BC_ROLE_PIPE);
        topo->sink = bc_sink_create(topo->plist, dtype, 1, BC_ROLE_PIPE);
    }
    return topo;
}

extern iskel_farm_t *
iskel_farm_create(bc_plist_t *pl, iskel_farm_fptr_t farmer,
        iskel_farm_fptr_t worker, iskel_farm_dmap_t *dmap,
        int mc) {
    int last;
    iskel_farm_t *farm;

    if (!bc_plist_isvalid(pl) ||
            !bc_plist_iselem(pl, bc_rank) ||
            !farmer || !worker || !dmap)
        return NULL;

    /* Farm should have atleast two stages: farmer and worker. */
    if (pl->count < 2)
        return NULL;
    last = pl->count - 1;

    if (!(farm = bc_malloc(iskel_farm_t, 1)))
        return NULL;

    /* Number of data farmed on each farm communication. */
    farm->num = last;

    /* With memory copy. */
    if (mc) {
        /* First process is always the farmer. */
        if (bc_rank == pl->plist[0]) {
            if (!bc_plist_self) {
                if (!(bc_plist_self = bc_malloc(bc_plist_t, 1))) {
                    bc_free(farm);
                    return NULL;
                }
                if (!(bc_plist_self->plist = bc_malloc(int, 1))) {
                    bc_free(bc_plist_self);
                    bc_free(farm);
                    return NULL;
                }
                bc_plist_self->plist[0] = bc_rank;
                bc_plist_self->valid = 0;
                bc_plist_self->count = 1;
                bc_plist_self->type = __BC_PLIST_INTERNAL;
                __bc_internal_flags |= BC_PLIST_SELF;
            }
            farm->plist = bc_plist_diff(pl, bc_plist_self);
            farm->src = bc_src_create(farm->plist, dmap[0].in,
                    BC_ROLE_COLLECT);
            farm->sink = bc_sink_create(farm->plist, dmap[0].out,
                    farm->num, BC_ROLE_SPREAD);
            farm->role = __ISKEL_FARM_FARMER;
            farm->fptr = farmer;
        } else {
            farm->plist = bc_plist_create(1, pl->plist[0]);
            farm->src = bc_src_create(farm->plist, dmap[1].in, BC_ROLE_PIPE);
            farm->sink = bc_sink_create(farm->plist, dmap[1].out, 1,
                    BC_ROLE_PIPE);
            farm->role = __ISKEL_FARM_WORKER;
            farm->fptr = worker;
        }
    } else {
        /* First process is always the farmer. */
        if (bc_rank == pl->plist[0]) {
            if (!bc_plist_self) {
                if (!(bc_plist_self = bc_malloc(bc_plist_t, 1))) {
                    bc_free(farm);
                    return NULL;
                }
                if (!(bc_plist_self->plist = bc_malloc(int, 1))) {
                    bc_free(bc_plist_self);
                    bc_free(farm);
                    return NULL;
                }
                bc_plist_self->plist[0] = bc_rank;
                bc_plist_self->valid = 0;
                bc_plist_self->count = 1;
                bc_plist_self->type = __BC_PLIST_INTERNAL;
                __bc_internal_flags |= BC_PLIST_SELF;
            }
            farm->plist = bc_plist_diff(pl, bc_plist_self);
            farm->src = bc_src_create(farm->plist, dmap[0].in, BC_ROLE_COLLECT);
            farm->sink = bc_sink_create(farm->plist, dmap[0].out, farm->num,
                    BC_ROLE_SPREAD);
            farm->role = __ISKEL_FARM_FARMER;
            farm->fptr = farmer;
        } else {
            farm->plist = bc_plist_create(1, pl->plist[0]);
            farm->src = bc_src_create(farm->plist, dmap[1].in, BC_ROLE_PIPE);
            farm->sink = bc_sink_create(farm->plist, dmap[1].out, 2,
                    BC_ROLE_PIPEN);
            farm->role = __ISKEL_FARM_WORKER;
            farm->fptr = worker;
        }
    }
    return farm;
}
