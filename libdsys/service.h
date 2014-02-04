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
#error "Please do not include 'service.h'; include 'dsys.h' instead."
#endif

#ifndef _BC_SERVICE_H
#define _BC_SERVICE_H

#include "common.h"
#include <unistd.h>

BEGIN_C_DECLS

typedef enum {
    __BC_SERVICE_GET = 0, /* Receive data, wait if not available. */
    __BC_SERVICE_KILL, /* Exit the thread. */
    __BC_SERVICE_ECHO, /* Echo as response. */
    __BC_SERVICE_COND_GET, /* Receive data if available, else return. */

    __BC_SERVICE_NUM
} bc_service_t;

typedef struct {
    bc_service_t code; /* Service code. */
    unsigned long tag; /* Link tag. */
    unsigned int dunits; /* Number of data units */
} bc_request_t;

extern size_t __bc_request_send(int fd, bc_service_t code,
        unsigned long tag, unsigned int dunits);
extern size_t __bc_request_recv(int fd, bc_request_t *req);

END_C_DECLS

#endif /* #define _BC_SERVICE_H */
