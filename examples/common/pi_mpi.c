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

/* Adapted from LAM-MPI implementation. */

#include <stdio.h>
#include <math.h>
#include <mpi.h>


/* Number of estimations. */
#define NUM_ITER 1000

int main(int argc, char *argv[])
{
  int iter, rank, size, i;
  double PI25DT = 3.141592653589793238462643;
  double mypi, pi, h, sum, x;
  double startwtime = 0.0, endwtime;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  for (iter = 2; iter < NUM_ITER; ++iter) {
    h = 1.0 / (double) iter;
    sum = 0.0;
	startwtime = MPI_Wtime();

    for (i = rank + 1; i <= iter; i += size) {
      x = h * ((double) i - 0.5);
      sum += 4.0 / (1.0 + x * x);
    }
    mypi = h * sum;
    
    MPI_Reduce(&mypi, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
      printf("%d points: PI is approximately %.16f, error = %.16f\n",
	     iter, pi, fabs(pi - PI25DT));
      endwtime = MPI_Wtime();
      printf("[%d] Wall clock time = %f\n", rank, endwtime - startwtime);
      fflush(stdout);
    } else {
      endwtime = MPI_Wtime();
      printf("[%d] Wall clock time = %f\n", rank, endwtime - startwtime);
	}
  }

  MPI_Finalize();
  return 0;
}
