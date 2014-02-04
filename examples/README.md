# Example programs

The following are example programs that demonstrate usage of the
`libdsys` API, Version 0.6.

1. `collect.c`
 Collect data produced by other processes.
 Demonstrates usage of `DSYS_ROLE_COLLECT`.

2. `dtype.c`
 Demonstrates creation/usage/destruction of custom data types.

3. `exchange.c`
 Demonstrates lock-step exchange of data between two process.

4. `farm.c`
 Demonstrates usage of `DSYS_ROLE_FARM` and `DSYS_ROLE_FARMN`.

5. `fft.c`
 Fast Fourier Transformation implementation.

6. `fft_iperm.c`
 Program for input permutation in the FFT. Produces the
 permutation as a visual image file.

7. `gauss_pipeline.c`
 Gaussian Elimination algorithm implementation
 with pipeline communications.

8. `gauss_replication.c`
 Gaussian Elimination algorithm implementation
 with replication communications.

9. `mandelbrot.c`
 Produces the Mandelbrot Set as a visual image file.

10. `matrix_gen.c`
 Generates two random matrices for testing `matrix_mul.c`.

11. `matrix_mul.c`
 Matrix Multiplications algorithm implementation.

12. `odd_even.c`
 Odd-Even Transposition sorting algorithm implementation.

13. `pi_dsys.c`
 `libdsys` implementation of the PI approximation.

14. `pi_mpi.c`
 MPI implementation of the PI approximation.

15. `reduce.c`
 Demonstrates usage of:

         1. `DSYS_ROLE_REDUCE_SUM`,
         2. `DSYS_ROLE_REDUCE_MUL`,
         3. `DSYS_ROLE_REDUCE_MIN`,
         4. `DSYS_ROLE_REDUCE_MAX`,
         5. `DSYS_ROLE_REDUCE_MINMAX`
         6. `DSYS_ROLE_REDUCE_OPT`

16. `replicate.c`
   Demonstrates usage of `DSYS_ROLE_REPLICATE`.

17. `spread.c`
   Demonstrates usage of `DSYS_ROLE_SPREAD`.
