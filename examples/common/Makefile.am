# Makefile.am -- Process this file with automake to produce Makefile.in
# Copyright (C) 2004, Gagarine Yaikhom
# Copyright (C) 2004, University of Edinburgh
#
# $Id: Makefile.am,v 1.4 2004/11/15 18:16:42 project Exp $
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

CC = mpicc

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = matrices.dat

INCLUDES = -I$(top_builddir) -I$(top_srcdir)

noinst_PROGRAMS = \
	odd_even \
	mandelbrot \
	fft_iperm \
	fft \
	matrix_gen \
	matrix_mul \
	gauss_pipeline \
	gauss_replicate \
	pi_dsys \
	pi_mpi \
	mva \
	mpi_mva \
	mandelbrot_refined

odd_even_SOURCES = odd_even.c
odd_even_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread

mandelbrot_SOURCES = mandelbrot.c
mandelbrot_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread -lm

fft_SOURCES = fft.c
fft_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread -lm

fft_iperm_SOURCES = fft_iperm.c
fft_iperm_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread -lm

matrix_gen_SOURCES = matrix_gen.c
matrix_gen_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread -lm

matrix_mul_SOURCES = matrix_mul.c
matrix_mul_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread -lm

gauss_pipeline_SOURCES = gauss_pipeline.c
gauss_pipeline_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread -lm

gauss_replicate_SOURCES = gauss_replicate.c
gauss_replicate_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread -lm

pi_dsys_SOURCES = pi_dsys.c
pi_dsys_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread -lm

pi_mpi_SOURCES = pi_mpi.c
pi_mpi_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread -lm

mva_SOURCES = mva.c
mva_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread

mpi_mva_SOURCES = mpi_mva.c
mpi_mva_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread

mandelbrot_refined_SOURCES = mandelbrot_refined.c
mandelbrot_refined_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread -lm

# End of file
