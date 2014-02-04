/***************************************************************************
 *  Copyright  2004  Gagarine Yaikhom
 *  Copyright  2004  University of Edinburgh
 *  s0231576@sms.ed.ac.uk
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

#ifndef __BC_SYS_COMPILE
#error "Please do not include 'common.h'; include 'dsys.h' instead."
#endif

#ifndef _BC_COMMON_H
#define _BC_COMMON_H

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif

#if STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#elif HAVE_STRINGS_H
#include <strings.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef __cplusplus
#define BEGIN_C_DECLS  extern "C" {
#define END_C_DECLS    }
#else
#define BEGIN_C_DECLS
#define END_C_DECLS
#endif

#ifdef __GNUC__
#ifndef const
#define const __const
#endif
#ifndef signed
#define signed __signed
#endif
#ifndef volatile
#define volatile __volatile
#endif
#else
#define __inline__
#ifdef __STDC__
#undef  signed
#define signed
#undef  volatile
#define volatile
#endif
#endif

#ifdef __STDC__
#define BC_STR(x) #x
#define BC_CONCAT(prefix, name) prefix##name
#else
#define BC_STR(x) "x"
#define BC_CONCAT(prefix, name) prefix/**/name
#endif

#define BC_NULL 0x00000000 /* No flags. */
#define BC_ERR  0x00000001 /* Display error messages. */

extern unsigned long __bc_internal_flags;

#include "error.h"

#endif /* _BC_COMMON_H */
