/***************************************************************************
 *  Saturday June 12 00:57:02 2004
 *  Copyright  2004  Gagarine Yaikhom
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

#include <math.h>
#include <stdio.h>
#include <mpi.h>
#include <dsys.h>

#define MEMCPY 1

static int comp(const void *a, const void *b) {
    if (*(double *) a == *(double *) b)
        return 0;
    else if (*(double *) a > *(double *) b)
        return 1;
    else return -1;
}

double lock_step_exchange_mpi(int n, size_t size) {
    int idx, i;
    char *data;
    double elapsed, *times;
    MPI_Datatype ntype;
    MPI_Status status;

    data = (char *) malloc(sizeof (char)*size);
    times = (double *) malloc(sizeof (double)*n);
    MPI_Type_contiguous(size, MPI_CHAR, &ntype);
    MPI_Type_commit(&ntype);

    if (bc_rank == 0)
        for (i = 0; i < n; i++) {
            memset(data, bc_rank + 'a', size);
            elapsed = bc_gettime_usec();
            MPI_Sendrecv_replace(data, 1, ntype, 1, 0, 1, 0,
                    MPI_COMM_WORLD, &status);
            times[i] = bc_gettime_usec() - elapsed;
        } else
        for (i = 0; i < n; i++) {
            memset(data, bc_rank + 'a', size);
            elapsed = bc_gettime_usec();
            MPI_Sendrecv_replace(data, 1, ntype, 0, 0, 0, 0,
                    MPI_COMM_WORLD, &status);
            times[i] = bc_gettime_usec() - elapsed;
        }

    MPI_Type_free(&ntype);

    /* Find the median. */
    qsort(times, n, sizeof (double), comp);
    idx = n / 2;
    if (n % 2) {
        elapsed = times[idx];
    } else {
        elapsed = (times[idx - 1] + times[idx + 1]) / 2;
    }
    free(data);
    free(times);
    return elapsed;
}

#if MEMCPY

double lock_step_exchange_bc(int n, size_t size) {
    int idx, i;
    char *data;
    bc_plist_t *other = NULL;
    bc_chan_t *src = NULL, *sink = NULL;
    bc_dtype_t *ntype;
    double elapsed, *times;

    data = (char *) malloc(sizeof (char)*size);
    times = (double *) malloc(sizeof (double)*n);
    ntype = bc_dtype_create(sizeof (char)*size);

    if (bc_rank == 0)
        other = bc_plist_create(1, 1);
    else
        other = bc_plist_create(1, 0);

    src = bc_src_create(other, ntype, BC_ROLE_PIPE);
    sink = bc_sink_create(other, ntype, 1, BC_ROLE_PIPE);

    for (i = 0; i < n; i++) {
        memset(data, bc_rank + 'a', size);
        elapsed = bc_gettime_usec();
        bc_put(sink, data, 1);
        bc_get(src, data, 1);
        times[i] = bc_gettime_usec() - elapsed;
    }

    bc_chan_destroy(src);
    bc_chan_destroy(sink);
    bc_plist_destroy(other);
    bc_dtype_destroy(ntype);

    /* Find the median. */
    qsort(times, n, sizeof (double), comp);
    idx = n / 2;
    if (n % 2) {
        elapsed = times[idx];
    } else {
        elapsed = (times[idx - 1] + times[idx + 1]) / 2;
    }
    free(data);
    free(times);
    return elapsed;
}
#else

double lock_step_exchange_bc(int n, size_t size) {
    int idx, i;
    char *data;
    bc_plist_t *other = NULL;
    bc_chan_t *src = NULL, *sink = NULL;
    bc_dtype_t *ntype;
    double elapsed, *times;

    data = (char *) malloc(sizeof (char)*size);
    times = (double *) malloc(sizeof (double)*n);
    ntype = bc_dtype_create(sizeof (char)*size);

    if (bc_rank == 0)
        other = bc_plist_create(1, 1);
    else
        other = bc_plist_create(1, 0);

    src = bc_src_create(other, ntype, BC_ROLE_PIPE);
    sink = bc_sink_create(other, ntype, 2, BC_ROLE_PIPEN);

    for (i = 0; i < n; i++) {
        memset(bc_vptr(sink, char), bc_rank + 'a', size);
        elapsed = bc_gettime_usec();
        bc_commit(sink);
        bc_get(src, data, 1);
        times[i] = bc_gettime_usec() - elapsed;
    }

    bc_chan_destroy(src);
    bc_chan_destroy(sink);
    bc_plist_destroy(other);
    bc_dtype_destroy(ntype);

    /* Find the median. */
    qsort(times, n, sizeof (double), comp);
    idx = n / 2;
    if (n % 2) {
        elapsed = times[idx];
    } else {
        elapsed = (times[idx - 1] + times[idx + 1]) / 2;
    }
    free(data);
    free(times);
    return elapsed;
}
#endif

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    bc_init(BC_ERR);

    printf("[%d] bc: %f usecs\n", bc_rank,
            lock_step_exchange_bc(1001, 8192));
    printf("[%d]  mpi: %f usecs\n", bc_rank,
            lock_step_exchange_mpi(1001, 8192));

    bc_final();
    MPI_Finalize();
    return 0;
}

