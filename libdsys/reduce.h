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
#error "Please do not include 'reduce.h'; include 'dsys.h' instead."
#endif

#ifndef _BC_REDUCE_H
#define _BC_REDUCE_H

#include "branch.h"
#include "common.h"
#include "dtype.h"
#include "plist.h"
#include "iface.h"

BEGIN_C_DECLS

extern int __bc_r_sum_reduce(bc_chan_t *bc, void *local, unsigned int du);
extern int __bc_r_mul_reduce(bc_chan_t *bc, void *local, unsigned int du);
extern int __bc_r_max_reduce(bc_chan_t *bc, void *local, unsigned int du);
extern int __bc_r_min_reduce(bc_chan_t *bc, void *local, unsigned int du);
extern int __bc_r_minmax_reduce(bc_chan_t *bc, void *local, unsigned int du);
extern int __bc_r_opt_reduce(bc_chan_t *bc, void *local, unsigned int du);

END_C_DECLS

#endif /* _BC_REDUCE_H */
