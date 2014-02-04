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

static int comp(const void *a, const void *b) {
    if (*(double *) a == *(double *) b)
        return 0;
    else if (*(double *) a > *(double *) b)
        return 1;
    else return -1;
}

double put_latency_bc_mc(int n, size_t size) {
    int idx, i;
    char *data;
    bc_plist_t *other = NULL;
    bc_chan_t *src = NULL, *sink = NULL;
    bc_dtype_t *ntype;
    double start, *times;

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

        /* Put data. */
        start = bc_gettime_usec();
        bc_put(sink, data, 1);
        times[i] = bc_gettime_usec() - start;

        /* Wait for next cycle. */
        bc_get(src, data, 1);
    }

    bc_chan_destroy(src);
    bc_chan_destroy(sink);
    bc_plist_destroy(other);
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

double put_latency_bc_nmc(int n, size_t size) {
    int idx, i;
    char *data;
    bc_plist_t *other = NULL;
    bc_chan_t *src = NULL, *sink = NULL;
    bc_dtype_t *ntype;
    double start, *times;

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
        /* Set data directly to the output buffer. */
        memset(bc_vptr(sink, char), bc_rank + 'a', size);

        /* Put data. */
        start = bc_gettime_usec();
        bc_commit(sink);
        times[i] = bc_gettime_usec() - start;

        /* Wait for next cycle. */
        bc_get(src, data, 1);
    }

    bc_chan_destroy(src);
    bc_chan_destroy(sink);
    bc_plist_destroy(other);
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

double put_latency_mpi_blk(int n, size_t size) {
    int idx, i;
    char *data;
    double start, *times;
    MPI_Datatype ntype;
    MPI_Status status;

    data = (char *) malloc(sizeof (char)*size);
    times = (double *) malloc(sizeof (double)*n);
    MPI_Type_contiguous(size, MPI_CHAR, &ntype);
    MPI_Type_commit(&ntype);

    if (bc_rank == 0)
        for (i = 0; i < n; i++) {
            memset(data, bc_rank + 'a', size);
            start = bc_gettime_usec();
            MPI_Send(data, 1, ntype, 1, 0, MPI_COMM_WORLD);
            times[i] = bc_gettime_usec() - start;
            MPI_Recv(data, 1, ntype, 1, 1, MPI_COMM_WORLD, &status);
        } else
        for (i = 0; i < n; i++) {
            MPI_Recv(data, 1, ntype, 0, 0, MPI_COMM_WORLD, &status);
            memset(data, bc_rank + 'a', size);

            start = bc_gettime_usec();
            MPI_Send(data, 1, ntype, 0, 1, MPI_COMM_WORLD);
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
    free(data);
    free(times);
    return start;
}

double put_latency_mpi_nblk(int n, size_t size) {
    int idx, i;
    char *data;
    double start, *times;
    MPI_Datatype ntype;
    MPI_Status status;
    MPI_Request req;

    data = (char *) malloc(sizeof (char)*size);
    times = (double *) malloc(sizeof (double)*n);
    MPI_Type_contiguous(size, MPI_CHAR, &ntype);
    MPI_Type_commit(&ntype);

    if (bc_rank == 0)
        for (i = 0; i < n; i++) {
            memset(data, bc_rank + 'a', size);
            start = bc_gettime_usec();
            MPI_Isend(data, 1, ntype, 1, 0, MPI_COMM_WORLD, &req);
            times[i] = bc_gettime_usec() - start;
            MPI_Wait(&req, MPI_STATUS_IGNORE);
            MPI_Recv(data, 1, ntype, 1, 1, MPI_COMM_WORLD, &status);
        } else
        for (i = 0; i < n; i++) {
            MPI_Recv(data, 1, ntype, 0, 0, MPI_COMM_WORLD, &status);
            memset(data, bc_rank + 'a', size);

            start = bc_gettime_usec();
            MPI_Isend(data, 1, ntype, 0, 1, MPI_COMM_WORLD, &req);
            times[i] = bc_gettime_usec() - start;
            MPI_Wait(&req, MPI_STATUS_IGNORE);
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
    free(data);
    free(times);
    return start;
}

int main(int argc, char *argv[]) {
    int i, limit = 21;
    MPI_Init(&argc, &argv);
    bc_init(BC_ERR);
    FILE *f;
    char fname[128];

    sprintf(fname, "%d%s.dat", bc_rank, "mc");
    f = fopen(fname, "w");
    for (i = 0; i < limit; i++) {
        fprintf(f, "%d %ld %f\n", i, 1L << i,
                put_latency_bc_mc(1001, 1L << i));
    }
    fclose(f);

    sleep(1);

    sprintf(fname, "%d%s.dat", bc_rank, "nmc");
    f = fopen(fname, "w");
    for (i = 0; i < limit; i++) {
        fprintf(f, "%d %ld %f\n", i, 1L << i,
                put_latency_bc_nmc(1001, 1L << i));
    }
    fclose(f);

    sleep(1);

    sprintf(fname, "%d%s.dat", bc_rank, "blk");
    f = fopen(fname, "w");
    for (i = 0; i < limit; i++) {
        fprintf(f, "%d %ld %f\n", i, 1L << i,
                put_latency_mpi_blk(1001, 1L << i));
    }
    fclose(f);

    sleep(1);

    sprintf(fname, "%d%s.dat", bc_rank, "nblk");
    f = fopen(fname, "w");
    for (i = 0; i < limit; i++) {
        fprintf(f, "%d %ld %f\n", i, 1L << i,
                put_latency_mpi_nblk(1001, 1L << i));
    }
    fclose(f);

    bc_final();
    MPI_Finalize();
    return 0;
}

