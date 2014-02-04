# Makefile.am -- Process this file with automake to produce Makefile.in
# Copyright (C) 2004, Gagarine Yaikhom
# Copyright (C) 2004, University of Edinburgh
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

EXTRA_DIST = test_and_generate

INCLUDES = -I$(top_builddir) -I$(top_srcdir)

noinst_PROGRAMS = \
	exchange \
	put_latency \
	get_latency \
	coll_bcast \
	coll_scatter \
	coll_gather \
	benchmark

exchange_SOURCES = exchange.c
exchange_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread -lm

put_latency_SOURCES = put_latency.c
put_latency_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread -lm

get_latency_SOURCES = get_latency.c
get_latency_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread -lm

coll_bcast_SOURCES = coll_bcast.c
coll_bcast_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread -lm

coll_scatter_SOURCES = coll_scatter.c
coll_scatter_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread -lm

coll_gather_SOURCES = coll_gather.c
coll_gather_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread -lm

benchmark_SOURCES = benchmark.c
benchmark_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread -lm

# End of file
