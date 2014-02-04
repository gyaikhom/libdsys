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
#error "Please do not include 'dtype.h'; include 'dsys.h' instead."
#endif

#ifndef _BC_DTYPE_H
#define _BC_DTYPE_H

#include "common.h"
#include "threads.h"

#include <sys/types.h>

#define bc_char        ( &__bc_char__ )
#define bc_uchar       ( &__bc_uchar__ )
#define bc_short       ( &__bc_short__ )
#define bc_ushort      ( &__bc_ushort__ )
#define bc_int         ( &__bc_int__ )
#define bc_uint        ( &__bc_uint__ )
#define bc_float       ( &__bc_float__ )
#define bc_long        ( &__bc_long__ )
#define bc_ulong       ( &__bc_ulong__ )
#define bc_double      ( &__bc_double__ )
#define bc_long_double ( &__bc_long_double__ )

BEGIN_C_DECLS

enum {
    BC_CHAR = 0,
    BC_UCHAR,
    BC_SHORT,
    BC_USHORT,
    BC_INT,
    BC_UINT,
    BC_FLOAT,
    BC_LONG,
    BC_ULONG,
    BC_DOUBLE,
    BC_LONG_DOUBLE,

    BC_DTYPE_CUST,
    BC_DTYPE_NUM
};

typedef struct bc_dtype_s {
    unsigned int code; /* Code for the data type */
    size_t bytes; /* Size in bytes */
    bc_mutex_t lock; /* Lock for reference counting. */
    unsigned int rc; /* Reference counter. */
} bc_dtype_t;

extern bc_dtype_t __bc_char__;
extern bc_dtype_t __bc_uchar__;
extern bc_dtype_t __bc_short__;
extern bc_dtype_t __bc_ushort__;
extern bc_dtype_t __bc_int__;
extern bc_dtype_t __bc_uint__;
extern bc_dtype_t __bc_float__;
extern bc_dtype_t __bc_long__;
extern bc_dtype_t __bc_ulong__;
extern bc_dtype_t __bc_double__;
extern bc_dtype_t __bc_long_double__;

#ifdef BC_SHOW_ERROR /* Display error messages. */
extern bc_dtype_t *__bc_dtype_create(size_t bytes, unsigned long line,
        const char *fname);
extern int __bc_dtype_destroy(bc_dtype_t *dtype, unsigned long line,
        const char *fname);
#define bc_dtype_create(bytes) \
        __bc_dtype_create((bytes), __LINE__, __FILE__);
#define bc_dtype_destroy(dtype) \
        __bc_dtype_destroy((dtype), __LINE__, __FILE__);
#else /* Display error messages. */
extern bc_dtype_t *bc_dtype_create(size_t bytes);
extern int bc_dtype_destroy(bc_dtype_t *dtype);
#endif /* Display error messages. */

extern int __bc_internal_dtype_destroy(bc_dtype_t *dtype);
#define __bc_internal_dtype_ref(dtype) \
        if ((dtype)->code == BC_DTYPE_CUST){ \
            bc_mutex_lock(&((dtype)->lock)); \
            (dtype)->rc++; \
            bc_mutex_unlock(&((dtype)->lock)); \
        }

END_C_DECLS

#endif /* _BC_DTYPE_H */
