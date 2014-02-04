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

#include "timing.h"

/* Get current time in seconds. */
double bc_gettime_sec(void) {
    struct timeval curtime;

    gettimeofday(&curtime, NULL);
    return (curtime.tv_sec + curtime.tv_usec * 1e-6);
}

/* Get current time in microseconds. */
double bc_gettime_usec(void) {
    struct timeval curtime;

    gettimeofday(&curtime, NULL);
    return (curtime.tv_sec * 1e6 + curtime.tv_usec);
}
