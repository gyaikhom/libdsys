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

#include "variable.h"
#include "mem.h"

/*
 * x = y (op) z
 */
void bc_variable_operation(bc_dtype_t *dtype, int mode,
        void *x, void *y, void *z) {
    switch (mode) {
        case BC_VARIABLE_ADD:
            switch (dtype->code) {
                case BC_INT:
                    *(int *) x = *(int *) y + *(int *) z;
                    break;
                case BC_FLOAT:
                    *(float *) x = *(float *) y + *(float *) z;
                    break;
                case BC_DOUBLE:
                    *(double *) x = *(double *) y + *(double *) z;
                    break;
                case BC_LONG:
                    *(long *) x = *(long *) y + *(long *) z;
                    break;
                case BC_LONG_DOUBLE:
                    *(long double *) x
                            = *(long double *) y + *(long double *) z;
                    break;
                case BC_SHORT:
                    *(short *) x = *(short *) y + *(short *) z;
                    break;
                case BC_USHORT:
                    *(unsigned short *) x
                            = *(unsigned short *) y + *(unsigned short *) z;
                    break;
                case BC_UINT:
                    *(unsigned int *) x
                            = *(unsigned int *) y + *(unsigned int *) z;
                    break;
                case BC_ULONG:
                    *(unsigned long *) x
                            = *(unsigned long *) y + *(unsigned long *) z;
                    break;
            }
            break;
        case BC_VARIABLE_MULTIPLY:
            switch (dtype->code) {
                case BC_INT:
                    *(int *) x = *(int *) y * *(int *) z;
                    break;
                case BC_FLOAT:
                    *(float *) x = *(float *) y * *(float *) z;
                    break;
                case BC_DOUBLE:
                    *(double *) x = *(double *) y * *(double *) z;
                    break;
                case BC_LONG:
                    *(long *) x = *(long *) y * *(long *) z;
                    break;
                case BC_LONG_DOUBLE:
                    *(long double *) x
                            = *(long double *) y * *(long double *) z;
                    break;
                case BC_SHORT:
                    *(short *) x = *(short *) y * *(short *) z;
                    break;
                case BC_USHORT:
                    *(unsigned short *) x
                            = *(unsigned short *) y * *(unsigned short *) z;
                    break;
                case BC_UINT:
                    *(unsigned int *) x
                            = *(unsigned int *) y * *(unsigned int *) z;
                    break;
                case BC_ULONG:
                    *(unsigned long *) x
                            = *(unsigned long *) y * *(unsigned long *) z;
                    break;
            }
            break;
        case BC_VARIABLE_MAX:
            switch (dtype->code) {
                case BC_INT:
                    *(int *) x = *(int *) y > *(int *) z
                            ? *(int *) y : *(int *) z;
                    break;
                case BC_FLOAT:
                    *(float *) x = *(float *) y > *(float *) z
                            ? *(float *) y : *(float *) z;
                    break;
                case BC_DOUBLE:
                    *(double *) x = *(double *) y > *(double *) z
                            ? *(double *) y : *(double *) z;
                    break;
                case BC_LONG:
                    *(long *) x = *(long *) y > *(long *) z
                            ? *(long *) y : *(long *) z;
                    break;
                case BC_LONG_DOUBLE:
                    *(long double *) x
                            = *(long double *) y > *(long double *) z
                            ? *(long double *) y : *(long double *) z;
                    break;
                case BC_SHORT:
                    *(short *) x = *(short *) y > *(short *) z
                            ? *(short *) y : *(short *) z;
                    break;
                case BC_USHORT:
                    *(unsigned short *) x
                            = *(unsigned short *) y > *(unsigned short *) z
                            ? *(unsigned short *) y : *(unsigned short *) z;
                    break;
                case BC_UINT:
                    *(unsigned int *) x
                            = *(unsigned int *) y > *(unsigned int *) z
                            ? *(unsigned int *) y : *(unsigned int *) z;
                    break;
                case BC_ULONG:
                    *(unsigned long *) x
                            = *(unsigned long *) y > *(unsigned long *) z
                            ? *(unsigned long *) y : *(unsigned long *) z;
                    break;
            }
            break;
        case BC_VARIABLE_MIN:
            switch (dtype->code) {
                case BC_INT:
                    *(int *) x = *(int *) y < *(int *) z
                            ? *(int *) y : *(int *) z;
                    break;
                case BC_FLOAT:
                    *(float *) x = *(float *) y < *(float *) z
                            ? *(float *) y : *(float *) z;
                    break;
                case BC_DOUBLE:
                    *(double *) x = *(double *) y < *(double *) z
                            ? *(double *) y : *(double *) z;
                    break;
                case BC_LONG:
                    *(long *) x = *(long *) y < *(long *) z
                            ? *(long *) y : *(long *) z;
                    break;
                case BC_LONG_DOUBLE:
                    *(long double *) x
                            = *(long double *) y < *(long double *) z
                            ? *(long double *) y : *(long double *) z;
                    break;
                case BC_SHORT:
                    *(short *) x = *(short *) y < *(short *) z
                            ? *(short *) y : *(short *) z;
                    break;
                case BC_USHORT:
                    *(unsigned short *) x
                            = *(unsigned short *) y < *(unsigned short *) z
                            ? *(unsigned short *) y : *(unsigned short *) z;
                    break;
                case BC_UINT:
                    *(unsigned int *) x
                            = *(unsigned int *) y < *(unsigned int *) z
                            ? *(unsigned int *) y : *(unsigned int *) z;
                    break;
                case BC_ULONG:
                    *(unsigned long *) x
                            = *(unsigned long *) y < *(unsigned long *) z
                            ? *(unsigned long *) y : *(unsigned long *) z;
                    break;
            }
            break;
    }
}

void bc_variable_create(bc_dtype_t *dtype, void **var) {
    switch (dtype->code) {
        case BC_INT:
            *var = bc_malloc(int, 1);
            break;
        case BC_FLOAT:
            *var = bc_malloc(float, 1);
            break;
        case BC_DOUBLE:
            *var = bc_malloc(double, 1);
            break;
        case BC_CHAR:
            *var = bc_malloc(char, 1);
            break;
        case BC_LONG:
            *var = bc_malloc(long, 1);
            break;
        case BC_LONG_DOUBLE:
            *var = bc_malloc(long double, 1);
            break;
        case BC_SHORT:
            *var = bc_malloc(short, 1);
            break;
        case BC_UCHAR:
            *var = bc_malloc(unsigned char, 1);
            break;
        case BC_USHORT:
            *var = bc_malloc(unsigned short, 1);
            break;
        case BC_UINT:
            *var = bc_malloc(unsigned int, 1);
            break;
        case BC_ULONG:
            *var = bc_malloc(unsigned long, 1);
            break;
    }
}

void bc_variable_assign(bc_dtype_t *dtype, void *var, long double value) {
    switch (dtype->code) {
        case BC_INT:
            *(int *) var = (int) value;
            break;
        case BC_FLOAT:
            *(float *) var = (float) value;
            break;
        case BC_DOUBLE:
            *(double *) var = (double) value;
            break;
        case BC_CHAR:
            *(char *) var = (char) value;
            break;
        case BC_LONG:
            *(long *) var = (long) value;
            break;
        case BC_LONG_DOUBLE:
            *(long double *) var = (long double) value;
            break;
        case BC_SHORT:
            *(short *) var = (short) value;
            break;
        case BC_UCHAR:
            *(unsigned char *) var = (unsigned char) value;
            break;
        case BC_USHORT:
            *(unsigned short *) var = (unsigned short) value;
            break;
        case BC_UINT:
            *(unsigned int *) var = (unsigned int) value;
            break;
        case BC_ULONG:
            *(unsigned long *) var = (unsigned long) value;
            break;
    }
}
