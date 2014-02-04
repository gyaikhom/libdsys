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

#include "branch.h"
#include "iface.h"
#include "pipe.h"
#include "reduce.h"
#include "replicate.h"
#include "farm.h"
#include "collect.h"
#include "spread.h"
#include "collect_any.h"

bc_iface_t const bc_iface[BC_ROLE_NUM] = {
    {NULL, NULL, NULL, NULL, NULL, NULL},
    {__bc_c_pipe, __bc_d_pipe, __bc_p_pipe, __bc_s_pipe, NULL, __bc_r_pipe},
    {__bc_c_pipen, __bc_d_pipen, __bc_p_pipen, NULL, __bc_s_pipen, __bc_r_pipe},
    {__bc_c_replicate, __bc_d_replicate, __bc_p_replicate,
            __bc_s_replicate, NULL, NULL},
    {__bc_c_replicaten, __bc_d_replicaten, __bc_p_replicaten,
            NULL, __bc_s_replicaten, NULL},
    {NULL, NULL, NULL, NULL, NULL, __bc_r_sum_reduce},
    {NULL, NULL, NULL, NULL, NULL, __bc_r_mul_reduce},
    {NULL, NULL, NULL, NULL, NULL, __bc_r_max_reduce},
    {NULL, NULL, NULL, NULL, NULL, __bc_r_min_reduce},
    {NULL, NULL, NULL, NULL, NULL, __bc_r_minmax_reduce},
    {NULL, NULL, NULL, NULL, NULL, __bc_r_opt_reduce},
    {__bc_c_farmn, __bc_d_farmn, __bc_p_farmn, NULL, __bc_s_farmn, NULL},
    {__bc_c_farmn, __bc_d_farmn, __bc_p_farmn, NULL, __bc_s_farmn, NULL},
    {NULL, NULL, NULL, NULL, NULL, __bc_r_collect},
    {__bc_c_spread, __bc_d_spread, __bc_p_spread, __bc_s_spread, NULL, NULL},
    {NULL, NULL, NULL, NULL, NULL, __bc_r_collect_any}
};

bc_reduce_opt_t bc_reduce_operator = NULL;
