/*
 * Mean Value Analysis algorithm with Schwitzer's approximation.
 *
 * Written by: Gagarine Yaikhom
 * School of Informaticws, University of Edinburgh
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <dsys.h>

#define DELAY 0
#define QUEUE 1

#define alloc_1d(V, T, N, X)										\
	if (!(V = (T *) calloc ((N), sizeof(T)))) {						\
		fprintf (stderr, "Error allocating %s", mem_names[(X)]);	\
		cleanup(1);													\
	} else allocs[(X)] = 1;

#define alloc_2d(V, T, N, X)										\
	if (!(V = (T **) calloc ((N), sizeof(T)))) {					\
		fprintf (stderr, "Error allocating %s", mem_names[(X)]);	\
		cleanup(1);													\
	} else allocs[(X)] = 1;
#define val(V,X,Y) V[X * nqueue + Y]
#define ptr(V,X,Y) &V[X * nqueue + Y]

enum {SERV_DEMAND, LOAD_VECTOR, VISIT_VECTOR, QUEUE_LENGTH,
	  CURRENT_QLEN, THROUGHPUT, RESIDENCE, QTYPE, NUM_ALLOCS};

int allocs[NUM_ALLOCS]; /* Memory allocation trace. */
int nclass;				/* Number of classes. */
int nqueue;				/* Number of queues. */
float epsilon = 0.0005; /* Relative error tolerance. */
char mem_names[8][24]={"service demand",
					   "load intensity vector",
					   "visitation vector",
					   "queue length estimate",
					   "current queue length",
					   "throughput",
					   "residence time",
					   "resource type"};

int *qtype;				/* Queue types. */
float *load_vect;		/* load intensity vector. */
float *visit_vect;		/* Queue visitation vector. */
float *throughput;		/* Class throughput. */
float *serv_demand;		/* Service demand. */
float *qlen_est;		/* Queue length estimate. */
float *qlen;			/* Current queue length estimate. */
float *res_time;		/* Residence times. */

static int cleanup(int flag) {
	if (allocs[SERV_DEMAND]) free(serv_demand);
	if (allocs[LOAD_VECTOR]) free(load_vect);
	if (allocs[VISIT_VECTOR]) free(visit_vect);
	if (allocs[QUEUE_LENGTH]) free(qlen_est);
	if (allocs[CURRENT_QLEN]) free(qlen);
	if (allocs[THROUGHPUT]) free(throughput);
	if (allocs[RESIDENCE]) free(res_time);
	if (allocs[QTYPE]) free(qtype);
	
	if (flag) exit(1);
	else return 0;
}

static int init(char *fname)
{
	int i, j;
	FILE *f;

	if (!(f = fopen (fname, "r"))) {
		perror("Opening input");
		exit(1);
	}
	if (fscanf(f, "%d %d %f\n", &nclass, &nqueue, &epsilon) == EOF) {
		perror("Reading input file");
		fclose(f);
		exit(1);
	}
	alloc_1d(qtype, int, nqueue, QTYPE);
	alloc_1d(load_vect, float, nclass, LOAD_VECTOR);
	alloc_1d(visit_vect, float, nclass, VISIT_VECTOR);
	alloc_1d(throughput, float, nclass, THROUGHPUT);
	alloc_1d(serv_demand, float, nclass*nqueue, SERV_DEMAND);
	alloc_1d(qlen_est, float, nclass*nqueue, QUEUE_LENGTH);
	alloc_1d(qlen, float, nclass*nqueue, CURRENT_QLEN);
	alloc_1d(res_time, float, nclass*nqueue, RESIDENCE);
	for (i = 0; i < nqueue; i++) fscanf(f, "%d", &qtype[i]);
	for (i = 0; i < nclass; i++) fscanf(f, "%f", &load_vect[i]);
	for (i = 0; i < nclass; i++) fscanf(f, "%f", &visit_vect[i]);
	for (i = 0; i < nclass; i++)
		for (j = 0; j < nqueue; j++)
			fscanf(f, "%f", ptr(serv_demand,i,j));
	if (bc_rank == 0) {
		printf ("  Number of classes : %d\n", nclass);
		printf ("Number of resources : %d\n", nqueue);
		printf ("    Error tolerance : %f\n", epsilon);
		printf ("\nService demand:\n");
		for (i = 0; i < nclass; i++) {
			printf (" class %d> ", i);
			for (j = 0; j < nqueue; j++)
				printf("%8.4f ", val(serv_demand,i,j));
			printf ("\n");
		}
		printf ("\nLoad intensity vector:\n");
		for (i = 0; i < nclass; i++) {
			printf (" class %d> ", i);
			printf ("%8.4f\n", load_vect[i]);
		}
		printf ("\nVisitation vector:\n");
		for (i = 0; i < nclass; i++) {
			printf (" class %d> ", i);
			printf ("%8.4f\n", visit_vect[i]);
		}
	}
	fclose(f);
	return 0;
}

static int isdone(bc_chan_t *src, bc_chan_t *sink)
{
	int i = bc_rank, j;
	float rerror = 0.0; /* Relative error. */
	float merror = 0.0; /* Maximum relative error. */
	float r_error;		/* Maximum error from remote. */

	for (j = 0; j < nqueue; j++) {
		rerror = fabs((val(qlen_est,i,j) - val(qlen,i,j))/val(qlen_est,i,j));
		if (rerror > merror) merror = rerror;
	}
	bc_put (sink, &merror, 1);
	bc_get (src, &r_error, 1);
	if (merror > r_error) merror = r_error;
	if (merror < epsilon) return 1;
	else return 0;
}

static int mva (int print_intermediate)
{
	int i = bc_rank, j;
	float one_less;		/* Queue length with one less class r request. */
	float sum_qlens;	/* Sum of the queue lengths except for the current class. */
	float sum_resis;	/* Sum of the residence times for the current class. */
	bc_chan_t *src_data, *sink_data;
	bc_chan_t *src_err, *sink_err;

	/* Estimate queue lengths. */ 
	for (j = 0; j < nqueue; j++)
		if (val(serv_demand,i,j) > 0)
			val(qlen_est,i,j) = (float)load_vect[i]/visit_vect[i];

	/* Create communication structure for data transfer. */
	sink_data = bc_sink_create (bc_plist_xall, bc_float, 10, BC_ROLE_REPLICATE);
	src_data = bc_src_create (bc_plist_xall, bc_float, BC_ROLE_REDUCE_SUM);

	/* Create communication structure for relative error. */
	sink_err = bc_sink_create (bc_plist_xall, bc_float, 10, BC_ROLE_REPLICATE);
	src_err = bc_src_create (bc_plist_xall, bc_float, BC_ROLE_REDUCE_MAX);

	
	/* During parallelisation, the following do loop is executed at
	 * each process, which represents a class. The interprocess 
	 * communications happens during the calculation of 'sum_qlens'
	 * and determination of the relative error. */
	do {
		for (j = 0; j < nqueue; j++)
			val(qlen,i,j) = val(qlen_est,i,j);	

		for (j = 0; j < nqueue; j++) {
			/* Queue length with one less class i request. */
			bc_put (sink_data, ptr(qlen,i,j), 1);
			bc_get (src_data, &sum_qlens, 1);
			one_less = (load_vect[i] - 1)/load_vect[i]*val(qlen,i,j) + sum_qlens;
			
			/* Class i residence time at queue j. */
			if (qtype[j] == DELAY)
				val(res_time,i,j) = val(serv_demand,i,j);
			else
				val(res_time,i,j) = val(serv_demand,i,j)*(1.0 + one_less);
		}
		/* Throughput for class i. */
		sum_resis = 0.0;
		for (j = 0; j < nqueue; j++)
			sum_resis += val(res_time,i,j);
		throughput[i] = load_vect[i]/sum_resis;
		
		/* Compute new estimates for the queue lengths. */
		for (j = 0; j < nqueue; j++)
			val(qlen_est,i,j) = throughput[i]*val(res_time,i,j);			
	} while (!isdone(src_err, sink_err));

	/* Destroy communication structure. */
	bc_chan_destroy (src_data); bc_chan_destroy (sink_data);
	bc_chan_destroy (src_err); bc_chan_destroy (sink_err);
	return 0;
}

static int print_results(void)
{
	int i = bc_rank, j;

	printf("\nQueue lengths:\n");
	printf (" class %d> ", i);
	for (j = 0; j < nqueue; j++)
		printf("%8.4f ", val(qlen,i,j));
	printf ("%8.4f\n", throughput[i]);
	return 0;
}

int main( int argc, char *argv[])
{
	MPI_Init (&argc, &argv);
	bc_init (BC_ERR|BC_PLIST_XALL);
	if (argc != 2) {
		fprintf (stderr,
				 "USAGE: mva <data file>\n\n"
				 "Data file format:\n\n"
				 "<num classes> <num queues> <error tolerance>\n"
				 "<queue type vector> (1 means QUEUE, 0 means DELAY)\n"
				 "<Load intensity vector>\n"
				 "<Visitation vector>\n"
				 "<service demand array>\n\nSee example file.\n");
		exit(0);
	}
	init(argv[1]);
 	mva(0);
 	print_results();
	cleanup(0);
 	bc_final();
	MPI_Finalize();
	return 0;
}
