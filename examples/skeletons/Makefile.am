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
	pipeline \
	pipeline_farm \
	all2all \
	farm \
	gauss_pipeline

pipeline_SOURCES = pipeline.c
pipeline_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread

pipeline_farm_SOURCES = pipeline_farm.c
pipeline_farm_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread

all2all_SOURCES = all2all.c
all2all_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread

farm_SOURCES = farm.c
farm_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread

gauss_pipeline_SOURCES = gauss_pipeline.c
gauss_pipeline_LDADD = $(top_builddir)/libdsys/libdsys.a -lpthread

# End of file
