/***************************************************************************
 * $Id$
 *
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
#include <mpi.h>
#include <dsys.h>

/* How many estimations. */
#define NUM_ITER 1000

int main(int argc, char *argv[])
{
  int iter, i;
  double PI25DT = 3.141592653589793238462643;
  double pi, h, sum, x;
  double startwtime = 0.0, endwtime;
  bc_chan_t *src = NULL, *sink = NULL;
  bc_plist_t *src_pl = NULL, *sink_pl = NULL;

  MPI_Init(&argc, &argv);
  bc_init(BC_ERR);

  if (bc_rank == 0) {
	  src_pl = bc_plist_create(-1, 0);
	  src = bc_src_create(src_pl, bc_double, BC_ROLE_REDUCE_SUM);
  } else {
	  sink_pl = bc_plist_create(1, 0);
	  sink = bc_sink_create(sink_pl, bc_double, 20000, BC_ROLE_PIPEN);
  }

  for (iter = 2; iter < NUM_ITER; ++iter) {
    h = 1.0 / (double) iter;
    sum = 0.0;
	startwtime = MPI_Wtime();

    for (i = bc_rank + 1; i <= iter; i += bc_size) {
      x = h * ((double) i - 0.5);
      sum += 4.0 / (1.0 + x * x);
    }
    
	if (bc_rank == 0) {
		bc_get(src, &pi, 1);
		pi += h * sum;
	} else {
		bc_var(sink, double) = h * sum;
		bc_commit(sink);
	}

    if (bc_rank == 0) {
      printf("%d points: PI is approximately %.16f, error = %.16f\n",
	     iter, pi, fabs(pi - PI25DT));
      endwtime = MPI_Wtime();
      printf("[%d] Wall clock time = %f\n", bc_rank, endwtime - startwtime);
      fflush(stdout);
    } else {
      endwtime = MPI_Wtime();
      printf("[%d] Wall clock time = %f\n", bc_rank, endwtime - startwtime);
	}
  }

  bc_final();
  MPI_Finalize();
  return 0;
}
