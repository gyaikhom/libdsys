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

AUX_DIST = \
	$(ac_aux_dir)/config.sub \
	$(ac_aux_dir)/config.guess \
	$(ac_aux_dir)/install-sh \
	$(ac_aux_dir)/missing \
	$(ac_aux_dir)/mkinstalldirs

EXTRA_DIST = VERSION bootstrap  m4/get_libdsys_version m4/libdsys_get_version.m4

AUTOMAKE_OPTIONS = gnu

SUBDIRS = libdsys examples

MAINTAINERCLEANFILES = Makefile Makefile.in aclocal.m4 configure $(AUX_DIST)

ACLOCAL = -I ./config

# End of file
