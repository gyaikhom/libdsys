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
 * Creates a Mandelbrot Set of 512x512 pixels, with complex range
 * from {-2, -2} to {2, 2}. There is one master, and seven slaves.
 * This will produce an XPM graphics file 'mandelbrot.xpm' which
 * can be view with gimp or xemacs.
 *
 * Usage: mpirun -c 8 mandelbrot
 */
 
#include <math.h>
#include <mpi.h>
#include <dsys.h>

/* Number of slaves, and slaves list. */
#define NUM_SLAVES  7
#define SLAVE_PLIST 1, 2, 3, 4, 5, 6, 7

/* Total number of pixels. */
#define PIX_ROWS    (512)
#define PIX_COLS    (512)
#define START       (-2.0)
#define END         (2.0)

/* Complex data type */
typedef struct complex_s{
    double real;
    double img;
} complex_t;

/* Define a new bc data type for sending complex data */
static bc_dtype_t *cmplx_t;

static long total_pix = PIX_ROWS*PIX_COLS;
static double range = END-START;

static int master(void);
static int slave(void);
static int calc_mandel_block(int count, complex_t *in, int *out);
static int calc_mandel_pixel(complex_t c);
static int generate_xpm(int *result);

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    bc_init(BC_ERR); 

    /* Create a complex data type. */
    cmplx_t = bc_dtype_create(sizeof(struct complex_s));
    
    switch (bc_rank) {
    case 0:
        master();
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        slave();
        break;
    default:
        printf ("[%d] I don't have to do anything\n", bc_rank);
    }
    
    /* Destroy the complex data type. */
    bc_dtype_destroy(cmplx_t);
    
    bc_final();
    MPI_Finalize();
    return 0;
}

/* Master function:
 * Sends block of data to the slaves, and wait for result.
 * Generates an XPM file giving the Mandelbrot set.
 */
int master(void)
{
    bc_chan_t *sink, *src;
    bc_plist_t *slaves;
    complex_t *plane, *p_ptr;
    int *result, *r_ptr;
    int i, j, offset, iter;
    double real, img, step;

    /* Allocate memory for the coordinates. */
    if (!(plane = (complex_t *) calloc(total_pix, sizeof(complex_t)))) {
		perror("Allocating memory");
		exit(1);
	}

    /* Allocate the memory for the results. */
    if (!(result = (int *) calloc(total_pix, sizeof(int)))) {
		free(plane);
		perror("Allocating memory");
		exit(1);
	}

    /* Create the slaves process list. */
	slaves = bc_plist_create(NUM_SLAVES, SLAVE_PLIST);
        
    /* Create the branching channels. */
    sink = bc_sink_create(slaves, cmplx_t, PIX_COLS, BC_ROLE_SPREAD);        
	src = bc_src_create(slaves, bc_int, BC_ROLE_COLLECT);
    
    /* Generate the coordinates. */
    img = START;
    step = range/PIX_COLS;
    for (i = 0; i < PIX_ROWS; i++) {
        real = START;
        for (j = 0; j < PIX_COLS; j++) {
            p_ptr = (plane + PIX_COLS * i + j);
            p_ptr->real = real;
            p_ptr->img = img;
            real += step;
        }
        img += step;
    }

    /* Farm the coordinates and harvest results. */
    p_ptr = plane;
    r_ptr = result;
    offset = NUM_SLAVES * PIX_COLS;
    iter = ceil(PIX_ROWS/NUM_SLAVES);
	for (i = 0; i < iter; i++) {
        /* Farm coordinates block. */
 	    bc_put(sink, p_ptr, PIX_COLS);

        /* Harvest results. */
  	    bc_get(src, r_ptr, PIX_COLS);
		
        p_ptr += offset;
        r_ptr += offset;
	}
    
    free(plane);
    bc_chan_destroy(sink);
	bc_chan_destroy(src);
    bc_plist_destroy(slaves);
    
    /* Generate the XPM file 'mandelbrot.xpm'. */
    generate_xpm(result);
    
    free(result);
    return 0;
}

/* Slave function:
 * Get block of coordinates from the master, produce the state of the
 * pixels and send the results back to the master. Continue until
 * not done.
 */
int slave(void)
{
    bc_chan_t *src, *sink;
    bc_plist_t *master;
    complex_t row[PIX_COLS];
    int i, result[PIX_COLS], iter;

    /* Create the master process list. */
    master = bc_plist_create(1, 0);
    
    /* Create the branching channels. */
	src = bc_src_create(master, cmplx_t, BC_ROLE_PIPE);
	sink = bc_sink_create(master, bc_int, PIX_COLS, BC_ROLE_PIPE);
    
    iter = ceil(PIX_ROWS/NUM_SLAVES);
	for (i = 0; i < iter; i++) {
        /* Get coordinates block. */
		bc_get(src, row, PIX_COLS);
        
        /* Process block. */
		calc_mandel_block(PIX_COLS, row, result);
        
        /* Send results block. */
		bc_put(sink, result, PIX_COLS);
	}
	bc_chan_destroy(src);
	bc_chan_destroy(sink);
    bc_plist_destroy(master);
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
        
        if (len_square > range)
            break;
        count++;
    } while (count < max_iter);
    return count;
}

/* Generate the XPM file */
int generate_xpm(int *result)
{
    FILE    *out;
    int     i, j, code;
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
        for (j = 0; j < PIX_COLS; j++) {
            code = *(result + i*PIX_COLS + j);
            fprintf(out, "%c", colour[code%16]);
        }
        fprintf(out, "%s", "\"");
    }
    fprintf(out, "%s", "};\n");
    
    fclose(out);
    return 0;
}
