# libdsys_get_version.m4
# M4 macro to get version information from VERSION file.
# Note: Adapted from LAM-MPI implementation.
#
# Copyright (C) 2004, Gagarine Yaikhom
# Copyright (C) 2004, University of Edinburgh 
#
# $Id$
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

AC_DEFUN([LIBDSYS_GET_VERSION],
[
libdsys_ver_dir="$1"
libdsys_ver_file="$2"
libdsys_ver_prog="sh $libdsys_ver_dir/get_libdsys_version $libdsys_ver_file"

libdsys_ver_run() {
  [eval] LIBDSYS_${2}=`$libdsys_ver_prog -${1}`
}

libdsys_ver_run f VERSION
libdsys_ver_run M MAJOR_VERSION
libdsys_ver_run m MINOR_VERSION
libdsys_ver_run r RELEASE_VERSION
libdsys_ver_run a ALPHA_VERSION
libdsys_ver_run b BETA_VERSION

unset libdsys_ver_dir libdsys_ver_file libdsys_ver_prog libdsys_ver_run

# Prevent multiple expansion
define([LIBDSYS_GET_VERSION], [])

])
