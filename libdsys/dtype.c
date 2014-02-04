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

#include "dtype.h"
#include "mem.h"
#include <stdio.h>

/* Predefined data types */
bc_dtype_t __bc_char__
        = {BC_CHAR, sizeof (char), PTHREAD_MUTEX_INITIALIZER, 1};
bc_dtype_t __bc_uchar__
        = {BC_UCHAR, sizeof (unsigned char), PTHREAD_MUTEX_INITIALIZER, 1};
bc_dtype_t __bc_short__
        = {BC_SHORT, sizeof (short), PTHREAD_MUTEX_INITIALIZER, 1};
bc_dtype_t __bc_ushort__
        = {BC_USHORT, sizeof (unsigned short), PTHREAD_MUTEX_INITIALIZER, 1};
bc_dtype_t __bc_int__
        = {BC_INT, sizeof (int), PTHREAD_MUTEX_INITIALIZER, 1};
bc_dtype_t __bc_uint__
        = {BC_UINT, sizeof (unsigned int), PTHREAD_MUTEX_INITIALIZER, 1};
bc_dtype_t __bc_float__
        = {BC_FLOAT, sizeof (float), PTHREAD_MUTEX_INITIALIZER, 1};
bc_dtype_t __bc_long__
        = {BC_LONG, sizeof (long), PTHREAD_MUTEX_INITIALIZER, 1};
bc_dtype_t __bc_ulong__
        = {BC_ULONG, sizeof (unsigned long), PTHREAD_MUTEX_INITIALIZER, 1};
bc_dtype_t __bc_double__
        = {BC_DOUBLE, sizeof (double), PTHREAD_MUTEX_INITIALIZER, 1};
bc_dtype_t __bc_long_double__
        = {BC_LONG_DOUBLE, sizeof (long double), PTHREAD_MUTEX_INITIALIZER, 1};

#ifdef BC_SHOW_ERROR
/*
 * Create a custom data type.
 *
 * bytes - Size of the custom data type in bytes (returned by sizeof()).
 * line  - Line number in the application code (used for debugging).
 * fname - Filename name of the application code.
 *
 * Return custom data type, or NULL if error.
 */
bc_dtype_t *__bc_dtype_create(size_t bytes, unsigned long line,
        const char *fname)
#else

bc_dtype_t *bc_dtype_create(size_t bytes)
#endif
{
    bc_dtype_t *dtype;

    if (bytes < 1) {
        bc_perror(BC_EIVAL, line, fname);
        return NULL;
    }
    if (!(dtype = bc_malloc(bc_dtype_t, 1))) {
        bc_perror(BC_EMEM, line, fname);
        return NULL;
    } else {
        dtype->code = BC_DTYPE_CUST;
        dtype->bytes = bytes;
        dtype->rc = 1; /* Initialise the reference count. */
        bc_mutex_init(&(dtype->lock));
        return dtype;
    }
}

#ifdef BC_SHOW_ERROR
/*
 * Destroy a custom data type.
 *
 * dtype - Custom data type to destroy.
 * line  - Line number in the application code (used for debugging).
 * fname - Filename name of the application code.
 *
 * Return BC_SUCCESS, or error code.
 */
int __bc_dtype_destroy(bc_dtype_t *dtype, unsigned long line,
        const char *fname)
#else

int bc_dtype_destroy(bc_dtype_t *dtype)
#endif
{
    if (!dtype) {
        bc_perror(BC_EIDTYPE, line, fname);
        return BC_EIDTYPE;
    }
    return __bc_internal_dtype_destroy(dtype);
}

/*
 * Internal function to destroy a custom data type.
 */
int __bc_internal_dtype_destroy(bc_dtype_t *dtype) {
    if (dtype->code != BC_DTYPE_CUST)
        return BC_SUCCESS;

    /* Acquire exclusive lock to the data type. */
    bc_mutex_lock(&(dtype->lock));

    /* Reduce the reference count. */
    dtype->rc--;

    /* Destroy if possible. */
    if (dtype->rc)
        bc_mutex_unlock(&(dtype->lock));
    else {
        bc_mutex_unlock(&(dtype->lock));
        bc_mutex_destroy(&(dtype->lock));
        bc_free(dtype);
    }
    return BC_SUCCESS;
}
