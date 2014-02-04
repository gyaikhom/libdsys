/***************************************************************************
 *  Saturday June 12 00:57:02 2004
 *  Copyright  2004  Gagarine Yaikhom
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

#include <math.h>
#include <stdio.h>
#include <mpi.h>
#include <dsys.h>

static int comp(const void *a, const void *b) {
    if (*(double *) a == *(double *) b)
        return 0;
    else if (*(double *) a > *(double *) b)
        return 1;
    else return -1;
}

double gather_bc(int n, size_t size) {
    int idx, i;
    char *data;
    bc_plist_t *other = NULL;
    bc_chan_t *src = NULL, *sink = NULL;
    bc_dtype_t *ntype;
    double start, *times;

    ntype = bc_dtype_create(sizeof (char)*size);
    times = (double *) malloc(sizeof (double)*n);

    if (bc_rank == 0) {
        data = (char *) malloc(sizeof (char)*size * bc_size);
        sink = bc_sink_create(bc_plist_xall, ntype, 2, BC_ROLE_REPLICATE);
        src = bc_src_create(bc_plist_xall, ntype, BC_ROLE_COLLECT);

        /* Warmup. */
        for (i = 0; i < 10; i++) {
            bc_get(src, data, 1);
            bc_put(sink, data, 1);
        }

        /* Evaluate. */
        for (i = 0; i < n; i++) {
            bc_get(src, data, 1);
            start = bc_gettime_usec();
            bc_get(src, data, 1);
            times[i] = bc_gettime_usec() - start;
            bc_put(sink, data, 1); /* Send acknowledgement. */
        }
    } else {
        data = (char *) malloc(sizeof (char)*size);
        other = bc_plist_create(1, 0);
        src = bc_src_create(other, ntype, BC_ROLE_PIPE);
        sink = bc_sink_create(other, ntype, 3, BC_ROLE_PIPE);

        memset(data, bc_rank + 'a', size);

        /* Warmup. */
        for (i = 0; i < 10; i++) {
            bc_put(sink, data, 1);
            bc_get(src, data, 1);
        }

        /* Evaluate. */
        for (i = 0; i < n; i++) {
            start = bc_gettime_usec();
            bc_put(sink, data, 1);
            times[i] = bc_gettime_usec() - start;
            bc_put(sink, data, 1);
            bc_get(src, data, 1); /* Wait for acknowledgement. */
        }
        bc_plist_destroy(other);
    }
    bc_chan_destroy(src);
    bc_chan_destroy(sink);
    bc_dtype_destroy(ntype);

    /* Find the median. */
    qsort(times, n, sizeof (double), comp);
    idx = n / 2;
    if (n % 2) {
        start = times[idx];
    } else {
        start = (times[idx - 1] + times[idx + 1]) / 2;
    }
    free(data);
    free(times);
    return start;
}

double gather_mpi(int n, size_t size) {
    int idx, i;
    void *sbuff, *rbuff;
    double start, *times;
    MPI_Datatype ntype;

    rbuff = (char *) malloc(sizeof (char)*size * bc_size);
    sbuff = (char *) malloc(sizeof (char)*size);
    times = (double *) malloc(sizeof (double)*n);

    MPI_Type_contiguous(size, MPI_CHAR, &ntype);
    MPI_Type_commit(&ntype);

    memset(sbuff, bc_rank + 'a', size);

    /* Warmup. */
    for (i = 0; i < 10; i++) {
        MPI_Gather(sbuff, 1, ntype, rbuff, 1, ntype, 0, MPI_COMM_WORLD);
    }

    /* Evaluate. */
    for (i = 0; i < n; i++) {
        start = bc_gettime_usec();
        MPI_Gather(sbuff, 1, ntype, rbuff, 1, ntype, 0, MPI_COMM_WORLD);
        times[i] = bc_gettime_usec() - start;
    }

    MPI_Type_free(&ntype);

    /* Find the median. */
    qsort(times, n, sizeof (double), comp);
    idx = n / 2;
    if (n % 2) {
        start = times[idx];
    } else {
        start = (times[idx - 1] + times[idx + 1]) / 2;
    }
    free(sbuff);
    free(rbuff);
    free(times);
    return start;
}

int main(int argc, char *argv[]) {
    int i, limit = 21;
    MPI_Init(&argc, &argv);
    bc_init(BC_ERR | BC_PLIST_XALL);
    FILE *f;
    char fname[128];

    sprintf(fname, "gather_bc_%d_%d.dat", bc_rank, bc_size);
    f = fopen(fname, "w");
    for (i = 0; i < limit; i++) {
        fprintf(f, "%d %ld %f\n", i, 1L << i, gather_bc(101, 1L << i));
    }
    fclose(f);

    sprintf(fname, "gather_mpi_%d_%d.dat", bc_rank, bc_size);
    f = fopen(fname, "w");
    for (i = 0; i < limit; i++) {
        fprintf(f, "%d %ld %f\n", i, 1L << i, gather_mpi(101, 1L << i));
    }
    fclose(f);

    bc_final();
    MPI_Finalize();
    return 0;
}

