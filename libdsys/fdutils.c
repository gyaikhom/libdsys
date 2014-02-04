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

#include "fdutils.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

int __bc_read_data(int fd, void *output, size_t bytes) {
    ssize_t bytes_read;

    do {
        bytes_read = recv(fd, output, bytes, MSG_WAITALL);
        bytes -= bytes_read;

        if (!bytes)
            return 0;

        output += bytes_read;
    } while (1);
    return 0;
}

int __bc_write_data(int fd, void *input, size_t bytes) {
    ssize_t bytes_written;

    do {
        bytes_written = send(fd, input, bytes, 0);
        bytes -= bytes_written;

        if (!bytes)
            return 0;

        input += bytes_written;
    } while (1);
    return 0;
}
