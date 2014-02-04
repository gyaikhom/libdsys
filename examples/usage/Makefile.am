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

INCLUDES = -I$(top_builddir) -I$(top_srcdir)

noinst_PROGRAMS = \
	exchange \
	reduce \
    replicate \
	farm \
	dtype \
	collect \
	spread \
	plist \
	collect_any

exchange_SOURCES = exchange.c
exchange_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread

reduce_SOURCES = reduce.c
reduce_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread

replicate_SOURCES = replicate.c
replicate_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread

farm_SOURCES = farm.c
farm_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread

dtype_SOURCES = dtype.c
dtype_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread

spread_SOURCES = spread.c
spread_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread

collect_SOURCES = collect.c
collect_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread

plist_SOURCES = plist.c
plist_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread

collect_any_SOURCES = collect_any.c
collect_any_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread

# End of file
