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

#include "base.h"
#include "common.h"
#include "mem.h"
#include "sockets.h"

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

static struct id_s {
    int rank;
    int type;
} id;

static int nagle_flag = 0; /* Disable Nagle Algorithm for the request sockets */
static int win_size = 128 * 1024; /* Set Socket Buffer to 128K */
static struct linger lng = {1, 10}; /* Linger setting on, 10 seconds */

static int __bc_create_listeners(int *req, int *data);
static int __bc_connect(int rank, int type);
static int __bc_accept(int l);

int __bc_socket_network_create(void) {
    register int size, i;
    int req, data, idx;

    if (!(bc_base_layer->snet = bc_malloc(bc_snet_t, 1)))
        return -1;

    size = bc_base_layer->mpi->size - 1;
    if (!(bc_base_layer->snet->slist = bc_malloc(bc_sockpair_t, size))) {
        bc_free(bc_base_layer->snet);
        return -1;
    }
    bc_base_layer->snet->count = size;

    __bc_create_listeners(&req, &data);

    /*
     * First serve all the connection requests for all the
     * processes who have rank before me, i.e. 'my_rank >'.
     * There are two connections to be granted for each process.
     */
    for (i = 0; i < bc_rank; i++) {
        __bc_accept(req);
        __bc_accept(data);
    }
    close(req);
    close(data);

    /*
     * Now, request connections from all the processes that
     * follow me.
     */
    id.rank = bc_rank;
    for (i = bc_rank + 1; i <= size; i++) {
        idx = __bc_rank2idx(i);
        bc_base_layer->snet->slist[idx].r_fd = __bc_connect(i, REQ_SOCK);
        bc_base_layer->snet->slist[idx].d_fd = __bc_connect(i, DATA_SOCK);
    }

#ifdef BC_DEBUG    
    printf("[%d] ", bc_rank);
    for (i = 0; i < size; i++)
        printf("(%d, %d)", bc_base_layer->snet->slist[i].r_fd,
                bc_base_layer->snet->slist[i].d_fd);
    printf("\n");
#endif

    return 0;
}

int __bc_socket_network_destroy(void) {
    register int i;

    /* Shutdown sockets from my side. */
    for (i = 0; i < bc_base_layer->snet->count; i++) {
        shutdown(bc_base_layer->snet->slist[i].r_fd, SHUT_WR);
        shutdown(bc_base_layer->snet->slist[i].d_fd, SHUT_RD);
    }

    /* Wait for the data serving thread manager to exit. */
    bc_join(bc_base_layer->tmgr->mgr, (void **) NULL);
    bc_free(bc_base_layer->tmgr);

    bc_free(bc_base_layer->snet->slist);
    bc_free(bc_base_layer->snet);
    return 0;
}

int __bc_connect(int rank, int type) {
    struct hostent *hostp;
    struct sockaddr_in remote;
    int fd;

    id.type = type;

    if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        return -1;

    setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *) &lng, sizeof (lng));

    if (!(hostp = gethostbyname(bc_base_layer->mpi->hnames[rank]))) {
        perror("Get host by name");
        return -1;
    }

    memset((void *) &remote, 0, sizeof (remote));
    memcpy((void *) &remote.sin_addr, hostp->h_addr, hostp->h_length);
    remote.sin_family = AF_INET;

    if (type == DATA_SOCK) {
        setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *) &win_size, sizeof (int));
        setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *) &win_size, sizeof (int));
        remote.sin_port = htons(__BC_BASE_PORT_NUMBER + bc_size + rank);
    } else {
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *) &nagle_flag,
                sizeof (nagle_flag));
        remote.sin_port = htons(__BC_BASE_PORT_NUMBER + rank);
    }

    while (connect(fd, (struct sockaddr *) &remote, sizeof (remote)) < 0);

    /* Send my rank and type of socket just created. */
    send(fd, &id, sizeof (struct id_s), 0);
    return fd;
}

int __bc_accept(int l) {
    struct sockaddr_in client;
    socklen_t addrlen = sizeof (client);
    fd_set rmask, mask;
    int retval, fd;

    FD_ZERO(&mask);
    FD_SET(l, &mask);

    do {
        rmask = mask;
        retval = select(l + 1, &rmask, NULL, NULL, NULL);

        if (retval < 0)
            continue;
        if (FD_ISSET(l, &rmask))
            break;
    } while (1);

    /* Accept a request for connection. */
    while ((fd = accept(l, (struct sockaddr *) &client, &addrlen)) < 0);

    /* Identify the rank of the client, and the socket type. */
    recv(fd, &id, sizeof (struct id_s), MSG_WAITALL);

    switch (id.type) {
        case REQ_SOCK:
            bc_base_layer->snet->slist[__bc_rank2idx(id.rank)].r_fd = fd;
            break;
        case DATA_SOCK:
            bc_base_layer->snet->slist[__bc_rank2idx(id.rank)].d_fd = fd;
            break;
    }
    return 0;
}

int __bc_create_listeners(int *req, int *data) {
    struct sockaddr_in inet;
    int optval = 1;

    if ((*req = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("Creating request listener");
        return -1;
    }
    if ((*data = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("Creating request listener");
        return -1;
    }

    /* Set the request socket options. */
    setsockopt(*req, SOL_SOCKET, SO_LINGER, (char *) &lng, sizeof (lng));
    setsockopt(*req, SOL_SOCKET, SO_REUSEADDR, (void *) &optval,
            sizeof (int));
    setsockopt(*req, IPPROTO_TCP, TCP_NODELAY, (char *) &nagle_flag,
            sizeof (nagle_flag));

    /* Set the data socket options. */
    setsockopt(*data, SOL_SOCKET, SO_LINGER, (char *) &lng, sizeof (lng));
    setsockopt(*data, SOL_SOCKET, SO_REUSEADDR, (void *) &optval,
            sizeof (int));
    setsockopt(*data, SOL_SOCKET, SO_SNDBUF, (char *) &win_size,
            sizeof (win_size));
    setsockopt(*data, SOL_SOCKET, SO_RCVBUF, (char *) &win_size,
            sizeof (win_size));

    memset((void *) &inet, 0, sizeof (inet));
    inet.sin_family = AF_INET;
    inet.sin_addr.s_addr = INADDR_ANY;
    inet.sin_port = htons(__BC_BASE_PORT_NUMBER + bc_rank);

    if (bind(*req, (struct sockaddr *) &inet, sizeof (inet)) < 0) {
        perror("Binding request listener");
        return -1;
    }
    if (listen(*req, bc_base_layer->mpi->size - 1) < 0) {
        close(*req);
        perror("Start request listener");
        return -1;
    }

    memset((void *) &inet, 0, sizeof (inet));
    inet.sin_family = AF_INET;
    inet.sin_addr.s_addr = INADDR_ANY;
    inet.sin_port = htons(__BC_BASE_PORT_NUMBER + bc_size + bc_rank);

    if (bind(*data, (struct sockaddr *) &inet, sizeof (inet)) < 0) {
        perror("Binding data listener");
        return -1;
    }
    if (listen(*data, bc_base_layer->mpi->size - 1) < 0) {
        close(*data);
        perror("Start data listener");
        return -1;
    }
    return 0;
}
