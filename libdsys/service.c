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

#include "service.h"
#include <sys/types.h>
#include <sys/socket.h>

size_t __bc_request_send(int fd, bc_service_t code, unsigned long tag,
        unsigned int dunits) {
    bc_request_t request, *req;
    size_t remain = sizeof (request), bytes_written;

    /* Pack request */
    request.code = code;
    request.tag = tag;
    request.dunits = dunits;
    req = &request;

    do {
        bytes_written = send(fd, req, remain, 0);
        remain -= bytes_written;

        if (!remain)
            return 0;

        req += bytes_written;
    } while (1);

    return 0;
}

size_t __bc_request_recv(int fd, bc_request_t *req) {
    size_t remain = sizeof (bc_request_t), bytes_read;

    do {
        bytes_read = recv(fd, req, remain, MSG_WAITALL);

        if (bytes_read == 0)
            return 0;

        remain -= bytes_read;
        if (!remain)
            return sizeof (bc_request_t);

        req += bytes_read;
    } while (1);
}
