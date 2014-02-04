# Makefile.am -- Process this file with automake to produce Makefile.in
# Copyright (C) 2004, Gagarine Yaikhom
# Copyright (C) 2004, University of Edinburgh
#
# $Id: Makefile.am,v 1.3 2004/10/29 15:08:21 project Exp $
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


MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_builddir) -I$(top_srcdir)

# User header file.
include_HEADERS = dsys.h

# Common header file.
common_includes = \
	base.h \
	branch.h \
	common.h \
	dtype.h \
	fdutils.h \
	ltab.h \
	variable.h \
	timing.h \
	bits.h \
	config.h

# Header files for process topology management.
topology_includes = \
	infmpi.h \
	plist.h

# Header files for network management.
network_includes = \
	sockets.h

# Header files for error menagement.
error_includes = \
	error.h

# Header files for buffer management.
buffer_includes = \
	iface.h \
	queue.h \
	pipe.h \
	reduce.h \
	replicate.h \
	farm.h \
	collect.h \
	spread.h \
	collect_any.h

# Header files for memory management.
memory_includes = \
	mem.h

# Header files for thread management.
threads_includes = \
	threads.h \
	service.h

# Header files for skeleton management.
skeletons_includes = \
	iskel.h

# Common source files.
common_sources = \
	base.c \
	branch.c \
	dtype.c \
	fdutils.c \
	ltab.c \
	variable.c \
	timing.c \
	bits.c


# Source files for process topology management.
topology_sources = \
	infmpi.c \
	plist.c

# Source files for network management.
network_sources = \
	sockets.c

# Source files for error menagement.
error_sources = \
	error.c

# Source files for buffer management.
buffer_sources = \
	iface.c \
	queue.c \
	pipe.c \
	reduce.c \
	replicate.c \
	farm.c \
	collect.c \
	spread.c \
	collect_any.c

# Source files for memory management.
memory_sources = \
	mem.c

# Source files for thread management.
threads_sources = \
	threads.c \
	service.c

# Source files for skeleton management.
skeletons_sources = \
	iskel.c

pkginclude_HEADERS = \
	$(topology_includes) $(network_includes) $(error_includes) \
	$(buffer_includes) $(memory_includes) $(threads_includes) \
	$(common_includes) $(skeletons_includes)

lib_LIBRARIES = libdsys.a

libdsys_a_SOURCES = \
	$(topology_sources) $(network_sources) $(error_sources) \
	$(buffer_sources) $(memory_sources) $(threads_sources) \
	$(common_sources) $(skeletons_sources)

# End of file.