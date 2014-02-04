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

#include "error.h"
#include "base.h"

#ifdef BC_SHOW_ERROR
char const bc_error_msgs[BC_NECODES][40] = {
    "Success",
    "libbc Layer could not be created",
    "Memory manager could not be initialised",
    "Node ranking could not be resolved",
    "Links hash table could not be created",
    "Socket network could not be created",
    "Process could not be multi-threaded",
    "Invalid branching channel",
    "Invalid function on branching channel",
    "Role not implemented",
    "Invalid consumer list",
    "Invalid producer list",
    "Invalid custom data type",
    "Could not allocate memory",
    "Invalid variable pointer",
    "Invalid argument",
    "Invalid process list"
};

void __bc_errmsg(int code, unsigned long line, const char *fname) {
    if (code < BC_ELAYER || code >= BC_NECODES)
        return;
    else if (code >= BC_ELAYER && code <= BC_ENODES)
        fprintf(stderr, "%s:%ld: %s.\n", fname, line, bc_error_msgs[code]);
    else
        fprintf(stderr, "[%d] %s:%ld: %s.\n", bc_rank, fname,
                line, bc_error_msgs[code]);
}
#endif
