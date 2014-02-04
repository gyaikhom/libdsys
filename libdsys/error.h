/***************************************************************************
 *  Copyright  2004  Gagarine Yaikhom
 *  Copyright  2004  University of Edinburgh
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

#ifndef __BC_SYS_COMPILE
#error "Please do not include 'error.h'; include 'dsys.h' instead."
#endif

#ifndef _BC_ERROR_H
#define _BC_ERROR_H

#include "common.h"

BEGIN_C_DECLS

typedef enum {
    BC_SUCCESS = 0,
    BC_ELAYER, /* Layer could not be created. */
    BC_EMMGER, /* Memory manager could not be initialised. */
    BC_ENODES, /* Node ranking could not be resolved. */
    BC_ELTAB, /* Links Look-up table could not be created. */
    BC_ESNET, /* Socket network could not be created. */
    BC_ETHREADS, /* Process could not be multi-threaded. */
    BC_EIBC, /* Invalid Branching Channel. */
    BC_EIFUNC, /* Invalid function on the branching channel. */
    BC_EIROLE, /* Role does not exists. */
    BC_EICONS, /* Invalid consumer list. */
    BC_EIPROD, /* Invalid producer list. */
    BC_EIDTYPE, /* Invalid data type. */
    BC_EMEM, /* Memory allocation error. */
    BC_EIPTR, /* Invalid pointer. */
    BC_EIVAL, /* Invalid value. */
    BC_EIPLIST, /* Invalid process list. */

    BC_NECODES /* Number of error codes. */
} bc_err_t;

#ifdef BC_SHOW_ERROR
extern char const bc_error_msgs[BC_NECODES][40];
extern void __bc_errmsg(int code, unsigned long line, const char *fname);
#define bc_perror(code, line, fname)									\
	if (__bc_internal_flags & BC_ERR) __bc_errmsg(code, line, fname);
#else
#define bc_perror(code, line, fname) /* Do nothing. */
#endif

END_C_DECLS

#endif /* _BC_ERROR_H */
