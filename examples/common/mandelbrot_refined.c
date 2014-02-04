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
 * There is one farmer, and seven workers.
 * This will produce an XPM graphics file 'mandelbrot.xpm' which
 * can be view with gimp, xemacs, or display.
 *
 * Usage: mpirun -c 8 mandelbrot_refined
 */
 
#include <math.h>
#include <mpi.h>
#include <dsys.h>

/* Number of workers, and workers list. */
#define NUM_WORKERS  7
#define WORKER_PLIST 1, 2, 3, 4, 5, 6, 7

/* Total number of pixels. */
#define PIX_ROWS    (1024)
#define PIX_COLS    (1024)
#define R_START     (-2.0)
#define R_END       (2.0)
#define I_START     (-2.0)
#define I_END       (2.0)

/* Complex data type */
typedef struct complex_s{
    double real;
    double img;
} complex_t;

typedef struct { int row; complex_t point[PIX_COLS]; } irow_t;
typedef struct { int row; int color[PIX_COLS]; } orow_t;

/* Define a new bc data type for sending complex data */
static bc_dtype_t *idtype, *odtype;

static double r_range = R_END-R_START;
static double i_range = I_END-I_START;

static int farmer(void);
static int worker(void);
static int calc_mandel_block(int count, complex_t *in, int *out);
static int calc_mandel_pixel(complex_t c);
static int generate_xpm(orow_t *result);

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    bc_init(BC_ERR); 

	if (bc_size < NUM_WORKERS+1) {
		if (bc_rank == 0)
			printf("ERROR: %d processes are required.\n"
				   "\n\tUSAGE: mpirun -c %d mandelbrot_refined\n\n",
				   NUM_WORKERS+1, NUM_WORKERS+1);
		bc_final();
		MPI_Finalize();
		return -1;
	}

    switch (bc_rank) {
    case 0:
        farmer();
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        worker();
        break;
    default:
        printf ("[%d] I don't have to do anything\n", bc_rank);
    }
    
	if (bc_rank == 0)
		printf("Execution successful...\n"
			   "Output generated into 'mandelbrot.xpm' image file.\n");
    bc_final();
    MPI_Finalize();
    return 0;
}

/* Farmer function:
 * Sends block of data to the workers, and wait for result.
 * Generates an XPM file giving the Mandelbrot set.
 */
int farmer(void)
{
    bc_chan_t *sink, *src;
    bc_plist_t *workers;
    int i, j, received;
    double real, img, r_step, i_step;
	orow_t output[PIX_ROWS];

    /* Create the workers process list. */
	workers = bc_plist_create(NUM_WORKERS, WORKER_PLIST);
        
    /* Create the branching channels. */
	idtype = bc_dtype_create(sizeof(irow_t));
	odtype = bc_dtype_create(sizeof(orow_t));
    sink = bc_sink_create(workers, idtype, PIX_ROWS + NUM_WORKERS + 1, BC_ROLE_FARMN);        
	src = bc_src_create(workers, odtype, BC_ROLE_COLLECT_ANY);
    
    /* Generate the coordinates. */
    img = I_START;
    r_step = r_range/PIX_COLS;
    i_step = i_range/PIX_ROWS;
    for (i = 0; i < PIX_ROWS; i++) {
        real = R_START;
		bc_vptr(sink, irow_t)->row = i;
        for (j = 0; j < PIX_COLS; j++) {
            bc_vptr(sink, irow_t)->point[j].real = real;
            bc_vptr(sink, irow_t)->point[j].img = img;
            real += r_step;
        }
        img += i_step;
		bc_commit(sink); /* Commit this row. */
    }
	j = PIX_ROWS;
    for (i = 0; i < NUM_WORKERS; i++) {
		bc_vptr(sink, irow_t)->row = -1; /* No more data available. */
		bc_commit(sink); /* Commit termination value. */
    }

	received = 0;
	while (received < PIX_ROWS) {
  	    bc_get(src, &output[received++], 1); /* Harvest results. */
	}

	bc_chan_destroy(src); bc_chan_destroy(sink);
	bc_dtype_destroy(idtype); bc_dtype_destroy(odtype);
    bc_plist_destroy(workers);
   
    /* Generate the XPM file 'mandelbrot.xpm'. */
    generate_xpm(output);
   
    return 0;
}

/* Worker function:
 * Get block of coordinates from the farmer, produce the state of the
 * pixels and send the results back to the farmer. Continue until
 * not done.
 */
int worker(void)
{
    bc_chan_t *src, *sink;
    bc_plist_t *farmer;
	irow_t input;
	orow_t output;
	int rows;

    /* Create the farmer process list. */
    farmer = bc_plist_create(1, 0);
    
    /* Create the branching channels. */
	idtype = bc_dtype_create(sizeof(irow_t));
	odtype = bc_dtype_create(sizeof(orow_t));
	src = bc_src_create(farmer, idtype, BC_ROLE_PIPE);
	sink = bc_sink_create(farmer, odtype, 2, BC_ROLE_PIPE);

	/* continue computation until no data available. */
	rows = 0;
	while (1) {
		bc_get(src, &input, 1); /* Get coordinates block. */
		if (input.row == -1) break;
        calc_mandel_block(PIX_COLS, input.point, output.color); /* Process block. */
        output.row = input.row;
		bc_put(sink, &output, 1); /* Send results block. */
		rows++;
	}
	printf("[%d] solved %d rows.\n", bc_rank, rows); 
	bc_chan_destroy(src);bc_chan_destroy(sink);
	bc_dtype_destroy(idtype);bc_dtype_destroy(odtype);
    bc_plist_destroy(farmer);
    return 0;
}

int
calc_mandel_block(int count, complex_t *in, int *out)
{
    int i;
    
    for (i = 0; i < count; i++)
        out[i] = calc_mandel_pixel(in[i]);

    return 0;
}

int calc_mandel_pixel(complex_t c)
{
    int         count = 0;
    int         max_iter = 255;
    complex_t   z;
    double      len_square, temp;
    
    z.real = z.img = 0.0;
    
    do {
        temp = z.real*z.real - z.img*z.img + c.real;
        z.img = 2.0*z.real*z.img + c.img;
        z.real = temp;
        len_square = z.real*z.real + z.img*z.img;
        
        if (len_square > 4.0)
            break;
        count++;
    } while (count < max_iter);
    return count;
}

/* Generate the XPM file */
int generate_xpm(orow_t *result)
{
    FILE    *out;
    int     i, j, k, code;
    char    colour[] = " .XoO+@#$%&*=-;:";
    
    out = fopen("mandelbrot.xpm", "w");
    
    fprintf(out, "%s", "/* XPM file: Mandelbrot Set */\n"
        "static char *mandelbrot[] = {\n");
    fprintf(out, "\"%d %d 16 1\",\n", PIX_ROWS, PIX_COLS); 
    fprintf(out, "%s", "\"  c #000000\",\n"
        "\". c #220000\",\n"
        "\"X c #440000\",\n"
        "\"o c #660000\",\n"
        "\"O c #880000\",\n"
        "\"+ c #aa0000\",\n"
        "\"@ c #cc0000\",\n"
        "\"# c #ee0000\",\n"
        "\"$ c #ee2200\",\n"
        "\"\% c #ee4400\",\n"
        "\"& c #ee6600\",\n"
        "\"* c #ee8800\",\n"
        "\"= c #eeaa00\",\n"
        "\"- c #eebb00\",\n"
        "\"; c #eedd00\",\n"
        "\": c #eeff00\"");
    
    for (i = 0; i < PIX_ROWS; i++) {
        fprintf(out, "%s", ",\n\"");
		for (k = 0; k < PIX_ROWS; k++)
			if (result[k].row == i) break;
        for (j = 0; j < PIX_COLS; j++) {
            code = result[k].color[j];
            fprintf(out, "%c", colour[code%16]);
        }
        fprintf(out, "%s", "\"");
    }
    fprintf(out, "%s", "};\n");
    
    fclose(out);
    return 0;
}
