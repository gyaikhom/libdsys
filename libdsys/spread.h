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
#error "Please do not include 'spread.h'; include 'dsys.h' instead."
#endif

#ifndef _BC_SPREAD_H
#define _BC_SPREAD_H

#include "branch.h"
#include "common.h"
#include "dtype.h"
#include "plist.h"

BEGIN_C_DECLS

extern bc_channel_t *__bc_c_spread(bc_plist_t *pl, bc_dtype_t *dtype,
        unsigned int du);
extern int __bc_d_spread(bc_channel_t *c);
extern int __bc_p_spread(bc_channel_t *c, void *local, unsigned int du);
extern int __bc_s_spread(bc_channel_t *c, int fd, unsigned int du,
        int tag, int cond_recv);

END_C_DECLS

#endif /* _BC_SPREAD_H */
