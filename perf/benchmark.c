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

/* Description:
 * This is used for benchmarking libbc performance. The following benchmarks
 * are provided by this program.
 *
 * (1) 'put' and 'get' Latencies:
 *     This provides the latency of putting data units into a branching
 *     channel, and getting data units out of a branching channel. 
 * (2) Round Trip Time (RTT):
 *     This provides the round trip time for a message to return to the sender.
 * (3) Lock-step Exchange Latency:
 *     This provides the time required for a successful data exchange.    
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <mpi.h> /* Used for initialization: Get Node Ranking. */
#include <dsys.h> /* libdsys header file. */

#define MAX_MSG_SIZE (  16) /* Maximum message size (exponential). */  
#define NUM_TRIALS   (   5) /* Number of trials. */
#define NUM_COMMS    (100L) /* Number of communications. */

enum {
    PUTGET = 0,
    SENDRECV,
    BC_RTT,
    MPI_RTT,
    BC_LSE,
    MPI_LSE,

    NUM_TEST
};

char testnames[NUM_TEST][10] = {
    "putget",
    "sendrecv",
    "bc_rtt",
    "mpi_rtt",
    "bc_lse",
    "mpi_lse"
};

static short master = 0; /* If I am the master process. */
static short putget_flag = 0; /* 'put' and 'get' latency test flag. */
static short sendrecv_flag = 0; /* 'send' and 'recv' latency test flag. */
static short bc_rtt_flag = 0; /* libbc Round Trip Time test flag. */
static short bc_lse_flag = 0; /* libbc Lock-step exchange latency test. */
static short mpi_rtt_flag = 0; /* MPI Round Trip Time test flag. */
static short mpi_lse_flag = 0; /* MPI Lock-step exchange latency test. */
static unsigned long median = NUM_TRIALS / 2 + 1; /* Num trials: always odd. */

FILE *outfile, *gnuplot;

static int compare(const void *x, const void *y);
static void print_settings(FILE *out);
static void print_gnuplot_header(FILE *out, char *filename, char *title);
static void print_gnuplot_command(FILE *out, char *filename, char *legend);
static int test(void);
static int run_test(int type);
static int testing_putget(int m);
static int testing_sendrecv(int m);
static int testing_bc_rtt(int m);
static int testing_mpi_rtt(int m);
static int testing_bc_lse(int m);
static int testing_mpi_lse(int m);

int main(int argc, char **argv) {
    char option;
    extern char *optarg;
    extern int optopt;

    MPI_Init(&argc, &argv);
    bc_init(BC_NULL);

    while ((option = getopt(argc, argv, "prld:PRLD:")) != -1) {
        switch (option) {
            case 'p': /* 'put' and 'get' latency test. */
                putget_flag = 1;
                break;
            case 'r': /* libbc Round Trip Time test. */
                bc_rtt_flag = 1;
                break;
            case 'l': /* libbc Lock-step exchange latency test. */
                bc_lse_flag = 1;
                break;
            case 'P': /* 'send' and 'recv' latency test. */
                sendrecv_flag = 1;
                break;
            case 'R': /* MPI Round Trip Time test. */
                mpi_rtt_flag = 1;
                break;
            case 'L': /* MPI Lock-step exchange latency test. */
                mpi_lse_flag = 1;
                break;
            case '?': /* Unrecognised option. */
                fprintf(stderr, "Unrecognised option -%c.\n", optopt);
                bc_final();
                MPI_Finalize();
                return 1;
            case ':': /* Value not provided. */
                fprintf(stderr, "Option -%c requires an operand.\n", optopt);
            default:
                bc_final();
                MPI_Finalize();
                return 1;
        }
    }

    if (bc_rank == 0) {
        master = 1;
        gnuplot = fopen("master.gnuplot", "w");
    } else {
        gnuplot = fopen("slave.gnuplot", "w");
    }

    print_settings(gnuplot);
    test();
    fclose(gnuplot);

    bc_final();
    MPI_Finalize();
    return 0;
}

int test(void) {
    if (putget_flag)
        run_test(PUTGET);

    if (sendrecv_flag)
        run_test(SENDRECV);

    if (bc_rtt_flag)
        run_test(BC_RTT);

    if (mpi_rtt_flag)
        run_test(MPI_RTT);

    if (bc_lse_flag)
        run_test(BC_LSE);

    if (mpi_lse_flag)
        run_test(MPI_LSE);

    return 0;
}

int run_test(int type) {
    int m;
    char filename[50];
    char legend[10];
    int (*fptr)(int);

    switch (type) {
        case PUTGET:
            fptr = testing_putget;
            if (master) {
                print_gnuplot_header(gnuplot, "put_latency.eps", "Put Latency");
                strcpy(legend, "bc\\\\_put()");
            } else {
                print_gnuplot_header(gnuplot, "get_latency.eps", "Get Latency");
                strcpy(legend, "bc\\\\_get()");
            }
            break;
        case BC_RTT:
            fptr = testing_bc_rtt;
            if (master) {
                print_gnuplot_header(gnuplot, "bc_rtt.eps", "Round Trip Time");
                strcpy(legend, "libbc");
            }
            break;
        case BC_LSE:
            fptr = testing_bc_lse;
            if (master)
                print_gnuplot_header(gnuplot,
                        "bc_lse_master.eps", "Lock-step Exchange Time");
            else
                print_gnuplot_header(gnuplot,
                        "bc_lse_slave.eps", "Lock-step Exchange Time");
            strcpy(legend, "libbc");
            break;
        case SENDRECV:
            fptr = testing_sendrecv;
            if (master) {
                print_gnuplot_header(gnuplot,
                        "send_latency.eps", "MPI\\\\_Send Latency");
                strcpy(legend, "MPI\\\\_Send()");
            } else {
                print_gnuplot_header(gnuplot,
                        "recv_latency.eps", "MPI\\\\_Recv Latency");
                strcpy(legend, "MPI\\\\_Recv()");
            }
            break;
        case MPI_RTT:
            fptr = testing_mpi_rtt;
            if (master) {
                print_gnuplot_header(gnuplot, "mpi_rtt.eps", "Round Trip Time");
                strcpy(legend, "MPI");
            }
            break;
        case MPI_LSE:
            fptr = testing_mpi_lse;
            if (master)
                print_gnuplot_header(gnuplot,
                        "mpi_lse_master.eps", "Lock-step Exchange Time");
            else
                print_gnuplot_header(gnuplot,
                        "mpi_lse_slave.eps", "Lock-step Exchange Time");
            strcpy(legend, "MPI");
            break;
        default:
            return 1;
    }

    if (master) {
        sprintf(filename, "%s_master.dat", testnames[type]);
        outfile = fopen(filename, "w");
    } else {
        sprintf(filename, "%s_slave.dat", testnames[type]);
        outfile = fopen(filename, "w");
    }
    fprintf(outfile, "# Filename: %s\n", filename);
    fprintf(outfile, "# Number of messages communicated: %ld\n# \n",
            NUM_COMMS);
    fprintf(outfile, "# Power \tMessage Size \tBuffer size \tLatency(usec)\n");

    if ((type != BC_RTT && type != MPI_RTT) || master)
        print_gnuplot_command(gnuplot, filename, legend);

    for (m = 0; m < MAX_MSG_SIZE; m++) {
        fptr(m);
        sleep(60);
    }

    fclose(outfile);
    return 0;
}

int testing_putget(int m) {
    unsigned long i, j;
    unsigned long msize;
    char *buff;
    double et[NUM_TRIALS], curr, start;

    msize = 1 << m;

    if (master) {
        bc_plist_t *cons;
        bc_chan_t *sink;

        buff = malloc(msize);
        cons = bc_plist_create(1, 1);
        sink = bc_sink_create(cons, bc_char, msize, BC_ROLE_PIPE);

        if (!buff || !cons || !sink) {
            printf("Failure");
            return -1;
        }
        for (i = 0; i < NUM_TRIALS; i++) {
            curr = 0.0;
            for (j = 0; j < NUM_COMMS; j++) {
                start = bc_gettime_usec();
                bc_put(sink, buff, msize);
                curr += bc_gettime_usec() - start;
            }
            et[i] = curr / NUM_COMMS;
        }
        bc_plist_destroy(cons);
        bc_chan_destroy(sink);
        free(buff);
        qsort(et, NUM_TRIALS, sizeof (double), compare);
        fprintf(outfile, "%5d\t\t%5ld\t\t%5ld\t\t%f\n", m, msize, msize,
                et[median]);
    } else {
        bc_plist_t *prod;
        bc_chan_t *src;

        buff = malloc(msize);
        prod = bc_plist_create(1, 0);
        src = bc_src_create(prod, bc_char, BC_ROLE_PIPE);

        if (!buff || !prod || !src) {
            printf("Failure");
            return -1;
        }
        for (i = 0; i < NUM_TRIALS; i++) {
            curr = 0.0;
            for (j = 0; j < NUM_COMMS; j++) {
                start = bc_gettime_usec();
                bc_get(src, buff, msize);
                curr += bc_gettime_usec() - start;
            }
            et[i] = curr / NUM_COMMS;
        }
        bc_plist_destroy(prod);
        bc_chan_destroy(src);
        free(buff);
        qsort(et, NUM_TRIALS, sizeof (double), compare);
        fprintf(outfile, "%5d\t\t%5ld\t\t%5ld\t\t%f\n", m, msize, msize,
                et[median]);
    }
    printf("Done: %d\n", m);
    return 0;
}

int testing_sendrecv(int m) {
    unsigned long i, j;
    unsigned long msize;
    char *buff;
    double et[NUM_TRIALS], curr, start;

    msize = 1 << m;

    if (master) {
        buff = malloc(msize);
        for (i = 0; i < NUM_TRIALS; i++) {
            curr = 0.0;
            for (j = 0; j < NUM_COMMS; j++) {
                start = bc_gettime_usec();
                MPI_Send(buff, msize, MPI_CHAR, 1, j, MPI_COMM_WORLD);
                curr += bc_gettime_usec() - start;
            }
            et[i] = curr / NUM_COMMS;
        }
        free(buff);
        qsort(et, NUM_TRIALS, sizeof (double), compare);
        fprintf(outfile, "%d\t\t%5ld\t\t-1\t\t%f\n", m, msize, et[median]);
    } else {
        MPI_Status status;
        buff = malloc(msize);
        for (i = 0; i < NUM_TRIALS; i++) {
            curr = 0.0;
            for (j = 0; j < NUM_COMMS; j++) {
                start = bc_gettime_usec();
                MPI_Recv(buff, msize, MPI_CHAR, 0, j, MPI_COMM_WORLD, &status);
                curr += bc_gettime_usec() - start;
            }
            et[i] = curr / NUM_COMMS;
        }
        free(buff);
        qsort(et, NUM_TRIALS, sizeof (double), compare);
        fprintf(outfile, "%d\t\t%5ld\t\t-1\t\t%f\n", m, msize, et[median]);
    }
    return 0;
}

int testing_bc_rtt(int m) {
    unsigned long i, j;
    unsigned long msize;
    char *buff;

    msize = 1 << m;

    if (master) {
        double et[NUM_TRIALS], curr, start;
        bc_plist_t *peer;
        bc_chan_t *sink, *src;

        buff = malloc(msize);
        peer = bc_plist_create(1, 1);
        sink = bc_sink_create(peer, bc_char, msize, BC_ROLE_PIPE);
        src = bc_src_create(peer, bc_char, BC_ROLE_PIPE);
        for (i = 0; i < NUM_TRIALS; i++) {
            curr = 0.0;
            for (j = 0; j < NUM_COMMS; j++) {
                start = bc_gettime_usec();
                bc_put(sink, buff, msize);
                bc_get(src, buff, msize);
                curr += bc_gettime_usec() - start;
            }
            et[i] = curr / NUM_COMMS;
        }
        bc_plist_destroy(peer);
        bc_chan_destroy(src);
        bc_chan_destroy(sink);
        free(buff);
        qsort(et, NUM_TRIALS, sizeof (double), compare);
        fprintf(outfile, "%d\t\t%5ld\t\t%5ld\t\t%f\n", m, msize, msize,
                et[median]);
    } else {
        bc_plist_t *peer;
        bc_chan_t *src, *sink;

        buff = malloc(msize);
        peer = bc_plist_create(1, 0);
        sink = bc_sink_create(peer, bc_char, msize, BC_ROLE_PIPE);
        src = bc_src_create(peer, bc_char, BC_ROLE_PIPE);
        for (i = 0; i < NUM_TRIALS; i++) {
            for (j = 0; j < NUM_COMMS; j++) {
                bc_get(src, buff, msize);
                bc_put(sink, buff, msize);
            }
        }
        bc_plist_destroy(peer);
        bc_chan_destroy(src);
        bc_chan_destroy(sink);
        free(buff);
    }
    return 0;
}

int testing_bc_lse(int m) {
    unsigned long i, j;
    unsigned long msize;
    char *buff;
    double et[NUM_TRIALS], curr, start;

    msize = 1 << m;

    if (master) {
        bc_plist_t *peer;
        bc_chan_t *sink, *src;

        buff = malloc(msize);
        peer = bc_plist_create(1, 1);
        sink = bc_sink_create(peer, bc_char, msize, BC_ROLE_PIPE);
        src = bc_src_create(peer, bc_char, BC_ROLE_PIPE);
        for (i = 0; i < NUM_TRIALS; i++) {
            curr = 0.0;
            for (j = 0; j < NUM_COMMS; j++) {
                start = bc_gettime_usec();
                bc_put(sink, buff, msize);
                bc_get(src, buff, msize);
                curr += bc_gettime_usec() - start;
            }
            et[i] = curr / NUM_COMMS;
        }
        bc_plist_destroy(peer);
        bc_chan_destroy(src);
        bc_chan_destroy(sink);
        free(buff);
        qsort(et, NUM_TRIALS, sizeof (double), compare);
        fprintf(outfile, "%d\t\t%5ld\t\t%5ld\t\t%f\n", m, msize, msize,
                et[median]);
    } else {
        bc_plist_t *peer;
        bc_chan_t *src, *sink;

        buff = malloc(msize);
        peer = bc_plist_create(1, 0);
        sink = bc_sink_create(peer, bc_char, msize, BC_ROLE_PIPE);
        src = bc_src_create(peer, bc_char, BC_ROLE_PIPE);
        for (i = 0; i < NUM_TRIALS; i++) {
            curr = 0.0;
            for (j = 0; j < NUM_COMMS; j++) {
                start = bc_gettime_usec();
                bc_put(sink, buff, msize);
                bc_get(src, buff, msize);
                curr += bc_gettime_usec() - start;
            }
            et[i] = curr / NUM_COMMS;
        }
        bc_plist_destroy(peer);
        bc_chan_destroy(src);
        bc_chan_destroy(sink);
        free(buff);
        qsort(et, NUM_TRIALS, sizeof (double), compare);
        fprintf(outfile, "%d\t\t%5ld\t\t%5ld\t\t%f\n", m, msize, msize,
                et[median]);
    }
    return 0;
}

int testing_mpi_rtt(int m) {
    unsigned long i, j;
    unsigned long msize;
    char *buff;
    MPI_Status status;

    msize = 1 << m;

    if (master) {
        double et[NUM_TRIALS], curr, start;
        buff = malloc(msize);
        for (i = 0; i < NUM_TRIALS; i++) {
            curr = 0.0;
            for (j = 0; j < NUM_COMMS; j++) {
                start = bc_gettime_usec();
                MPI_Send(buff, msize, MPI_CHAR, 1, j, MPI_COMM_WORLD);
                MPI_Recv(buff, msize, MPI_CHAR, 1, j, MPI_COMM_WORLD, &status);
                curr += bc_gettime_usec() - start;
            }
            et[i] = curr / NUM_COMMS;
        }
        free(buff);
        qsort(et, NUM_TRIALS, sizeof (double), compare);
        fprintf(outfile, "%d\t\t%5ld\t\t-1\t\t%f\n", m, msize, et[median]);
    } else {
        buff = malloc(msize);
        for (i = 0; i < NUM_TRIALS; i++) {
            for (j = 0; j < NUM_COMMS; j++) {
                MPI_Recv(buff, msize, MPI_CHAR, 0, j, MPI_COMM_WORLD, &status);
                MPI_Send(buff, msize, MPI_CHAR, 0, j, MPI_COMM_WORLD);
            }
        }
        free(buff);
    }
    return 0;
}

int testing_mpi_lse(int m) {
    unsigned long i, j;
    unsigned long msize;
    char *buff;
    MPI_Status status;
    double et[NUM_TRIALS], curr, start;

    msize = 1 << m;

    if (master) {
        buff = malloc(msize);
        for (i = 0; i < NUM_TRIALS; i++) {
            curr = 0.0;
            for (j = 0; j < NUM_COMMS; j++) {
                start = bc_gettime_usec();
                MPI_Sendrecv(buff, msize, MPI_CHAR, 1, j,
                        buff, msize, MPI_CHAR, 1, j,
                        MPI_COMM_WORLD, &status);
                curr += bc_gettime_usec() - start;
            }
            et[i] = curr / NUM_COMMS;
        }
        free(buff);
        qsort(et, NUM_TRIALS, sizeof (double), compare);
        fprintf(outfile, "%d\t\t%5ld\t\t-1\t\t%f\n", m, msize, et[median]);
    } else {
        buff = malloc(msize);
        for (i = 0; i < NUM_TRIALS; i++) {
            curr = 0.0;
            for (j = 0; j < NUM_COMMS; j++) {
                start = bc_gettime_usec();
                MPI_Sendrecv(buff, msize, MPI_CHAR, 0, j,
                        buff, msize, MPI_CHAR, 0, j,
                        MPI_COMM_WORLD, &status);
                curr += bc_gettime_usec() - start;
            }
            et[i] = curr / NUM_COMMS;
        }
        free(buff);
        qsort(et, NUM_TRIALS, sizeof (double), compare);
        fprintf(outfile, "%d\t\t%5ld\t\t-1\t\t%f\n", m, msize, et[median]);
    }
    return 0;
}

int compare(const void *x, const void *y) {
    if (*(double *) x == *(double *) y)
        return 0;
    else {
        if (*(double *) x < *(double *) y)
            return -1;
        else
            return 1;
    }
}

void print_settings(FILE *out) {
    fprintf(out,
            "# Script for plotting graphs.\n"
            "# Automatically generated by '%s'.\n"
            "# Date: %s\n"
            "# Time: %s\n# \n"
            "# libbc tests:\n"
            "#     'put' and 'get' latency test : %s\n"
            "#             Round Trip Time test : %s\n"
            "#  Lock-step exchange latency test : %s\n# \n",
            __FILE__, __DATE__, __TIME__,
            putget_flag ? "ON" : "OFF",
            bc_rtt_flag ? "ON" : "OFF",
            bc_lse_flag ? "ON" : "OFF"
            );
    fprintf(out,
            "# MPI tests:\n"
            "#     'put' and 'get' latency test : %s\n"
            "#             Round Trip Time test : %s\n"
            "#  Lock-step exchange latency test : %s\n# \n",
            sendrecv_flag ? "ON" : "OFF",
            mpi_rtt_flag ? "ON" : "OFF",
            mpi_lse_flag ? "ON" : "OFF"
            );
    fflush(out);
}

void print_gnuplot_header(FILE *out, char *filename, char *title) {
    fprintf(out,
            "\n\nset size 0.82, 0.82\n"
            /*  			"set yrange [0.0:100.0]\n" */
            /*  			"set xrange [1:65536]\n" */
            "set term postscript eps enhanced color\n"
            "set output \"%s\"\n"
            "set title \"%s\"\n"
            "set border 15 lw 0.2\n"
            "set logscale x 2\n"
            "set format x \"%s\"\n"
            "set key left top\n"
            "set xlabel \"Message size (bytes)\"\n"
            "set ylabel \"Time ({/Symbol m}-seconds)\" 1.0,0.0\n"
            "plot ", filename, title, "2^{%L}");
}

void print_gnuplot_command(FILE *out, char *filename, char *legend) {
    fprintf(out, "'%s' using 1:4 title \"%s\" with linespoints\n",
            filename, legend);
}
