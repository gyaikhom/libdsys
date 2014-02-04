g/***************************************************************************
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


#include "common.h"
#include "base.h"
#include "fdutils.h"
#include "iface.h"
#include "mem.h"
#include "queue.h"

bc_qiface_t bc_qiface[__BC_QTYPE_NUM] = {
    {NULL, NULL, NULL, NULL, NULL},
    {__bc_c_mc121, __bc_d_mc121, __bc_p_mc121, __bc_s_mc121, NULL},
    {__bc_c_nmc121, __bc_d_nmc121, __bc_p_nmc121, NULL, __bc_s_nmc121},
    {__bc_c_mc12m, __bc_d_mc12m, __bc_p_mc12m, __bc_s_mc12m, NULL},
    {__bc_c_nmc12m, __bc_d_nmc12m, __bc_p_nmc12m, NULL, __bc_s_nmc12m},
    {__bc_c_farmn_q, __bc_d_farmn_q, __bc_p_farmn_q, NULL, __bc_s_farmn_q}
};

static bc_nmc121_t *__bc_internal_nmc121_create(int du, size_t bytes);
static int __bc_internal_nmc121_destroy(bc_nmc121_t *q);
static int __bc_internal_nmc121_put(bc_nmc121_t *q, int p);
static int __bc_internal_nmc121_send(bc_nmc121_t *q, int fd, bc_vptr_t *vptr,
        int cond_recv);

static bc_nmc12m_t *__bc_internal_nmc12m_create(int du, size_t bytes);
static int __bc_internal_nmc12m_destroy(bc_nmc12m_t *q);
static int __bc_internal_nmc12m_put(bc_nmc12m_t *q, int p);
static int __bc_internal_nmc12m_send(bc_nmc12m_t *q, int fd, bc_vptr_t *vptr,
        int cond_recv);

static bc_farmn_q_t *__bc_internal_farmn_q_create(int du, size_t bytes);
static int __bc_internal_farmn_q_destroy(bc_farmn_q_t *q);
static int __bc_internal_farmn_q_put(bc_farmn_q_t *q, int p);
static int __bc_internal_farmn_q_send(bc_farmn_q_t *q, int fd,
        bc_vptr_t *vptr, int cond_recv);

void __bc_send_sorry_message(int fd) {
    int available = 0;
    __bc_write_data(fd, &available, sizeof (int));
}

void __bc_send_preamble_message(int fd) {
    int available = 1;
    __bc_write_data(fd, &available, sizeof (int));
}

bc_mc121_t *__bc_mc121_create(bc_plist_t *pl, size_t bytes) {
    bc_mc121_t *q;

    if (!(q = bc_malloc(bc_mc121_t, 1)))
        return NULL;
    if (!(q->buff_start = bc_malloc(void, bytes))) {
        bc_free(q);
        return NULL;
    }
    q->size = bytes;
    q->bytes_inbuff = 0;
    q->free_start = q->buff_start;
    q->data_start = q->buff_start;
    q->buff_end = q->buff_start + bytes;
    bc_mutex_init(&(q->lock));
    bc_cond_init(&(q->no_suff));
    return q;
}

bc_queue_t *__bc_c_mc121(bc_plist_t *pl, int du, size_t szdu) {
    bc_queue_t *q;

    if (!(q = bc_malloc(bc_queue_t, 1)))
        return NULL;
    if (!(q->mc121 = __bc_mc121_create(pl, du * szdu))) {
        bc_free(q);
        return NULL;
    }
    return q;
}

int __bc_mc121_destroy(bc_mc121_t *q) {
    bc_free(q->buff_start);
    bc_free(q);
    return 0;
}

int __bc_d_mc121(bc_queue_t *q) {
    __bc_mc121_destroy(q->mc121);
    bc_free(q);
    return 0;
}

int __bc_mc121_put(bc_mc121_t *q, void *local, size_t bytes, int tag) {
    bc_mutex_lock(&(q->lock));
    do {
        switch (__bc_putq_mc121(q, local, bytes)) {
            case 0:
                bc_mutex_unlock(&(q->lock));
                return 0;
            case -1:
                bc_cond_wait(&(q->no_suff), &(q->lock));
                break;
        }
    } while (1);
}

int __bc_p_mc121(bc_queue_t *q, void *local, size_t bytes, int tag) {
    return __bc_mc121_put(q->mc121, local, bytes, 0);
}

int __bc_mc121_send(bc_mc121_t *q, int fd, size_t bytes, int tag,
        int cond_recv) {
    bc_mutex_lock(&(q->lock));
    do {
        switch (__bc_sendq_mc121(q, fd, bytes, cond_recv)) {
            case 0:
                bc_mutex_unlock(&(q->lock));
                return 0;
            case -1:
                if (cond_recv) {
                    __bc_send_sorry_message(fd);
                    bc_mutex_unlock(&(q->lock));
                    return 0;
                } else
                    bc_cond_wait(&(q->no_suff), &(q->lock));
                break;
        }
    } while (1);
}

int __bc_s_mc121(bc_queue_t *q, int fd, size_t bytes, int tag,
        int cond_recv) {
    return __bc_mc121_send(q->mc121, fd, bytes, 0, cond_recv);
}

int __bc_putq_mc121(bc_mc121_t *q, void *local, size_t bytes) {
    if ((q->size - q->bytes_inbuff) < bytes)
        return -1;
    else {
        size_t upper_free;

        /* Calculate free bytes in upper block */
        upper_free = q->buff_end - q->free_start;

        /* Enough space in upper block. Note that we don't
         * use '>=' here because, if the free_start pointer 
         * reaches the upper bound of the buffer, it should
         * go back to the start of the buffer.
         */
        if (upper_free > bytes) {
            memcpy(q->free_start, local, bytes);
            q->free_start += bytes;
        } else {
            void *input_ptr;
            size_t bytes_remaining;

            memcpy(q->free_start, local, upper_free);
            bytes_remaining = bytes - upper_free;
            if (bytes_remaining > 0) {
                input_ptr = local + upper_free; /* Set source start */
                memcpy(q->buff_start, input_ptr, bytes_remaining);
                q->free_start = q->buff_start + bytes_remaining;
            } else {
                q->free_start = q->buff_start;
            }
        }
        q->bytes_inbuff += bytes;

        /* Signal all thread waiting for data */
        bc_cond_signal(&(q->no_suff));
        return 0;
    }
}

int __bc_sendq_mc121(bc_mc121_t *q, int fd, size_t bytes, int cond_recv) {
    /* 
     * The comparison only (as opposed to subtraction and comparison)
     * makes the get() faster than the put. This is deliberately done
     * because put() is local memory access, and get involves remote
     * process.
     */
    if (q->bytes_inbuff < bytes)
        return -1;
    else {
        size_t upper_data;

        /* Calculate data bytes in upper block */
        upper_data = q->buff_end - q->data_start;

        /* Enough data in upper block */
        if (upper_data > bytes) {
            if (cond_recv) __bc_send_preamble_message(fd);
            __bc_write_data(fd, q->data_start, bytes);
            q->data_start += bytes;
        } else {
            size_t bytes_remaining;

            if (cond_recv) __bc_send_preamble_message(fd);
            __bc_write_data(fd, q->data_start, upper_data);
            bytes_remaining = bytes - upper_data;
            if (bytes_remaining > 0) {
                __bc_write_data(fd, q->buff_start, bytes_remaining);
                q->data_start = q->buff_start + bytes_remaining;
            } else {
                q->data_start = q->buff_start;
            }
        }
        q->bytes_inbuff -= bytes;

        /* Signal all thread waiting for space */
        bc_cond_signal(&(q->no_suff));
        return 0;
    }
}

static int __bc_destroy_queue_list(bc_mc12m_t *q);

int __bc_destroy_queue_list(bc_mc12m_t *q) {
    bc_mc12m_t *temp;

    while (q) {
        temp = q;
        q = temp->next;
        __bc_mc121_destroy(temp->mc121);
        bc_free(temp);
    }

    return 0;
}

bc_queue_t *__bc_c_mc12m(bc_plist_t *pl, int du, size_t bytes) {
    bc_queue_t *q;
    bc_mc12m_t *temp, *tail = NULL;
    register int i;
    size_t s;

    if (!(q = bc_malloc(bc_queue_t, 1)))
        return NULL;

    q->mc12m = NULL;

    /*
     * Create individual queue for each process in the process list.
     */
    s = du*bytes;
    for (i = 0; i < pl->count; i++) {
        if (!(temp = bc_malloc(bc_mc12m_t, 1))) {
            __bc_destroy_queue_list(q->mc12m);
            bc_free(q);
            return NULL;
        }
        if (!(temp->mc121 = __bc_mc121_create(NULL, s))) {
            bc_free(temp);
            __bc_destroy_queue_list(q->mc12m);
            bc_free(q);
            return NULL;
        }
        temp->tag = pl->plist[i];

        /*
         * Because we do not know the order in which individual
         * queues will be accessed, we make the case of 'spread'
         * pattern to be efficient. Hence, creation of queue list
         * may be more expensive, but accessing the queue is more
         * efficient. Also, creation is done only once.
         */
        temp->next = NULL;
        if (!q->mc12m) /* First node. */
            q->mc12m = temp;
        else
            tail->next = temp;
        tail = temp;
    }

    return q;
}

int __bc_d_mc12m(bc_queue_t *q) {
    __bc_destroy_queue_list(q->mc12m);
    bc_free(q);
    return 0;
}

int __bc_p_mc12m(bc_queue_t *q, void *local, size_t bytes, int tag) {
    bc_mc12m_t *temp = q->mc12m;

    while (temp) {
        if (temp->tag == tag)
            goto process;
        temp = temp->next;
    }
    return -1;

process:
    bc_mutex_lock(&(temp->mc121->lock));
    do {
        switch (__bc_putq_mc121(temp->mc121, local, bytes)) {
            case 0:
                bc_mutex_unlock(&(temp->mc121->lock));
                return 0;
            case -1:
                bc_cond_wait(&(temp->mc121->no_suff), &(temp->mc121->lock));
                break;
        }
    } while (1);
}

int __bc_s_mc12m(bc_queue_t *q, int fd, size_t bytes, int tag,
        int cond_recv) {
    bc_mc12m_t *temp = q->mc12m;

    while (temp) {
        if (temp->tag == tag)
            goto process;
        temp = temp->next;
    }
    return -1;

process:
    bc_mutex_lock(&(temp->mc121->lock));
    do {
        switch (__bc_sendq_mc121(temp->mc121, fd, bytes, cond_recv)) {
            case 0:
                bc_mutex_unlock(&(temp->mc121->lock));
                return 0;
            case -1:
                if (cond_recv) {
                    __bc_send_sorry_message(fd);
                    bc_mutex_unlock(&(temp->mc121->lock));
                    return 0;
                } else
                    bc_cond_wait(&(temp->mc121->no_suff), &(temp->mc121->lock));
                break;
        }
    } while (1);
}

bc_queue_t *__bc_c_nmc121(bc_plist_t *pl, int du, size_t szdu) {
    bc_queue_t *q;

    if (!(q = bc_malloc(bc_queue_t, 1)))
        return NULL;
    if (!(q->nmc121 = __bc_internal_nmc121_create(du, szdu))) {
        bc_free(q);
        return NULL;
    }
    return q;
}

int __bc_d_nmc121(bc_queue_t *q) {
    __bc_internal_nmc121_destroy(q->nmc121);
    bc_free(q);
    return 0;
}

int __bc_p_nmc121(bc_queue_t *q, void *local, size_t bytes, int tag) {
    return __bc_internal_nmc121_put(q->nmc121, tag);
}

int __bc_s_nmc121(bc_queue_t *q, int fd, size_t bytes, bc_vptr_t *vptr,
        int cond_recv) {
    return __bc_internal_nmc121_send(q->nmc121, fd, vptr, cond_recv);
}

bc_nmc121_t *__bc_internal_nmc121_create(int du, size_t bytes) {
    bc_nmc121_t *q;
    size_t s;

    if (!(q = (bc_nmc121_t *) bc_malloc(bc_nmc121_t, 1))) {
        perror("Creating buffer");
        return NULL;
    }

    s = bytes*du;
    if (!(q->start = (int *) bc_malloc(char, s))) {
        perror("Allocating buffer space");
        bc_free(q);
        return NULL;
    }
    q->end = q->start + s;
    q->vptr.var = q->start;
    q->size = bytes;
    q->inbuff = 0;
    q->du = du;

    bc_cond_init(&(q->cond));
    bc_mutex_init(&(q->lock));

    return q;
}

int __bc_internal_nmc121_destroy(bc_nmc121_t *q) {
    if (!q)
        return -1;
    bc_free(q->start);
    bc_cond_destroy(&(q->cond));
    bc_mutex_destroy(&(q->lock));
    bc_free(q);
    return 0;
}

int __bc_internal_nmc121_put(bc_nmc121_t *q, int c) {
    bc_mutex_lock(&(q->lock));
    printf("Putting\n");
    if (q->inbuff == q->du)
        bc_cond_wait(&(q->cond), &(q->lock));
    q->inbuff++;
    q->vptr.var += q->size;
    if (q->vptr.var == q->end)
        q->vptr.var = q->start;
    bc_cond_signal(&(q->cond));
    bc_mutex_unlock(&(q->lock));
    printf("Put\n");
    return 0;
}

int __bc_internal_nmc121_send(bc_nmc121_t *q, int fd, bc_vptr_t *vptr,
        int cond_recv) {
    bc_mutex_lock(&(q->lock));
    printf("Sending...\n");
    if (!q->inbuff || (vptr->var == q->vptr.var)) {
        if (cond_recv) {
            __bc_send_sorry_message(fd);
            bc_mutex_unlock(&(q->lock));
            return 0;
        } else
            bc_cond_wait(&(q->cond), &(q->lock));
    }

    if (cond_recv) __bc_send_preamble_message(fd);
    __bc_write_data(fd, vptr->var, q->size);

    vptr->var += q->size;
    if (vptr->var == q->end)
        vptr->var = q->start;

    q->inbuff--;
    bc_cond_signal(&(q->cond));
    bc_mutex_unlock(&(q->lock));
    printf("Sent\n");
    return 0;
}

bc_queue_t *__bc_c_nmc12m(bc_plist_t *pl, int du, size_t szdu) {
    bc_queue_t *q;

    if (!(q = bc_malloc(bc_queue_t, 1)))
        return NULL;
    if (!(q->nmc12m = __bc_internal_nmc12m_create(du, szdu))) {
        bc_free(q);
        return NULL;
    }
    return q;
}

int __bc_d_nmc12m(bc_queue_t *q) {
    __bc_internal_nmc12m_destroy(q->nmc12m);
    bc_free(q);
    return 0;
}

int __bc_p_nmc12m(bc_queue_t *q, void *local, size_t bytes, int tag) {
    return __bc_internal_nmc12m_put(q->nmc12m, tag);
}

int __bc_s_nmc12m(bc_queue_t *q, int fd, size_t bytes, bc_vptr_t *vptr,
        int cond_recv) {
    return __bc_internal_nmc12m_send(q->nmc12m, fd, vptr, cond_recv);
}

bc_nmc12m_t *__bc_internal_nmc12m_create(int du, size_t bytes) {
    bc_nmc12m_t *q;
    size_t s;
    register int i;

    if (!(q = (bc_nmc12m_t *) bc_malloc(bc_nmc12m_t, 1))) {
        perror("Creating buffer");
        return NULL;
    }

    s = bytes*du;
    if (!(q->start = (int *) bc_malloc(char, s))) {
        bc_free(q);
        return NULL;
    }
    q->end = q->start + s;

    s = sizeof (unsigned short)*du;
    if (!(q->rc = (unsigned short *) bc_malloc(char, s))) {
        bc_free(q->start);
        bc_free(q);
        return NULL;
    }

    for (i = du - 1; i; i--)
        q->rc[i] = 0;

    q->vptr.var = q->start;
    q->vptr.rcidx = 0;
    q->size = bytes;
    q->du = du;

    bc_cond_init(&(q->cond));
    bc_mutex_init(&(q->lock));

    return q;
}

int __bc_internal_nmc12m_destroy(bc_nmc12m_t *q) {
    register int i;

    if (!q)
        return -1;

    for (i = q->du; i; i--)
        if (q->rc[i])
            return 0;

    bc_free(q->start);
    bc_free(q->rc);
    bc_cond_destroy(&(q->cond));
    bc_mutex_destroy(&(q->lock));
    bc_free(q);
    return 0;
}

/*
 * A single buffer is shared by the producer and all the data serving
 * threads associated with each consumer. Variable pointer (points to
 * a safe location within the buffer) and reference counder index is
 * maintained by each thread, producer and data serving threads. There
 * is a reference counter associated with each data unit location within
 * the buffer, which is shared by all threads. The unique rc index is
 * point to the effective rc for the particular thread.
 *
 *
 * Put:
 *
 * First lock the queue. Update the variable pointer associated with
 * the buffer (this should always succeed because put() cannot return
 * until variable points to a safe location. Then, set the resource
 * counter to the effective number of consumers. Now, check if it is
 * safe to return. This means that the location pointed, if resource
 * counter index is updated, should be a valid location. Valid location
 * means, data can be safely overwritten which means, resource counter
 * for that location should be zero. Until it is so, get into a loop.
 * Within the loop, if it is not safe, we brodcast on the conditional
 * variable associated with the queue and wait on this C.V.. If it is 
 * safe, we update the resource counter index, on the C.V., unlock the
 * mutex, and return safely.
 *
 * The conditional variable is used to signal buffer change (input and
 * output) between all the sharing threads.
 */
int __bc_internal_nmc12m_put(bc_nmc12m_t *q, int c) {
    int i;

    bc_mutex_lock(&(q->lock));

    /* Update variable pointer. */
    q->vptr.var += q->size;
    if (q->vptr.var == q->end)
        q->vptr.var = q->start;

    /* Set reference counter. */
    q->rc[q->vptr.rcidx] = c;

    /* Check if it is safe to return. */
    i = q->vptr.rcidx + 1;
    i %= q->du;
    while (1) {
        if (q->rc[i] == 0) {
            q->vptr.rcidx = i;
            bc_cond_broadcast(&(q->cond));
            bc_mutex_unlock(&(q->lock));
            return 0;
        }
        bc_cond_broadcast(&(q->cond));
        bc_cond_wait(&(q->cond), &(q->lock));
    }
    return 0;
}

/*
 * Get:
 *
 * First lock the queue. Check if data is ready for *ME*. If data is 
 * ready, the rc at rc index should be > 0, and this rc index should 
 * not equal the one associated with the producer's rc index. This is
 * to avoid the same thread removing data from the same location more
 * than once. If these condition are not satisfied, we wait on the
 * C.V. until the buffer gets updated.
 *
 * If it is safe to access the buffer, we send the data to the remote
 * computation thread. Then, we update the variable pointer, and update
 * rc and rc index. We brodcast on the C.V. to signal buffer change.
 * Finally, the mutex is unlocked and function returns safely.
 */
int __bc_internal_nmc12m_send(bc_nmc12m_t *q, int fd, bc_vptr_t *vptr,
        int cond_recv) {
    bc_mutex_lock(&(q->lock));

    /* Is data ready? */
    while (1) {
        if (q->rc[vptr->rcidx] && (q->vptr.rcidx != vptr->rcidx))
            break;
        if (cond_recv) {
            __bc_send_sorry_message(fd);
            bc_mutex_unlock(&(q->lock));
            return 0;
        } else
            bc_cond_wait(&(q->cond), &(q->lock));
    }

    /* Send the data. */
    if (cond_recv) __bc_send_preamble_message(fd);
    __bc_write_data(fd, vptr->var, q->size);

    /* Update link variable pointer. */
    vptr->var += q->size;
    if (vptr->var == q->end)
        vptr->var = q->start;

    /* Update rc and rc index. */
    q->rc[vptr->rcidx]--;
    vptr->rcidx++;
    vptr->rcidx %= q->du;

    /* Signal queue change. */
    bc_cond_broadcast(&(q->cond));
    bc_mutex_unlock(&(q->lock));

    return 0;
}

bc_queue_t *__bc_c_farmn_q(bc_plist_t *pl, int du, size_t szdu) {
    bc_queue_t *q;

    if (!(q = bc_malloc(bc_queue_t, 1)))
        return NULL;
    if (!(q->nmcus = __bc_internal_farmn_q_create(du, szdu))) {
        bc_free(q);
        return NULL;
    }
    return q;
}

int __bc_d_farmn_q(bc_queue_t *q) {
    __bc_internal_farmn_q_destroy(q->nmcus);
    bc_free(q);
    return 0;
}

int __bc_p_farmn_q(bc_queue_t *q, void *local, size_t bytes, int tag) {
    return __bc_internal_farmn_q_put(q->nmcus, tag);
}

int __bc_s_farmn_q(bc_queue_t *q, int fd, size_t bytes, bc_vptr_t *vptr,
        int cond_recv) {
    return __bc_internal_farmn_q_send(q->nmcus, fd, vptr, cond_recv);
}

bc_farmn_q_t *__bc_internal_farmn_q_create(int du, size_t bytes) {
    bc_farmn_q_t *q;
    size_t s;
    register int i;

    if (!(q = bc_malloc(bc_farmn_q_t, 1))) {
        perror("Creating buffer");
        return NULL;
    }

    s = bytes*du;
    if (!(q->start = bc_malloc(char, s))) {
        bc_free(q);
        return NULL;
    }
    q->end = q->start + s;

    s = sizeof (unsigned short)*du;
    if (!(q->rc = bc_malloc(unsigned short, s))) {
        bc_free(q->start);
        bc_free(q);
        return NULL;
    }

    for (i = du - 1; i; i--)
        q->rc[i] = 0;

    q->pptr.var = q->start;
    q->pptr.rcidx = 0;
    q->cptr.var = q->start;
    q->cptr.rcidx = 0;

    q->size = bytes;
    q->du = du;

    bc_cond_init(&(q->cond));
    bc_mutex_init(&(q->lock));

    return q;
}

int __bc_internal_farmn_q_destroy(bc_farmn_q_t *q) {
    register int i;

    if (!q)
        return -1;

    for (i = q->du; i; i--)
        if (q->rc[i])
            return 0;

    bc_free(q->start);
    bc_free(q->rc);
    bc_cond_destroy(&(q->cond));
    bc_mutex_destroy(&(q->lock));
    bc_free(q);
    return 0;
}

/*
 * A single buffer is shared by the producer and all the data serving
 * threads associated with each consumer. Variable pointer (points to
 * a safe location within the buffer) and reference counder index is
 * maintained by each thread, producer and data serving threads. There
 * is a reference counter associated with each data unit location within
 * the buffer, which is shared by all threads. The unique rc index is
 * point to the effective rc for the particular thread.
 *
 *
 * Put:
 *
 * First lock the queue. Update the variable pointer associated with
 * the buffer (this should always succeed because put() cannot return
 * until variable points to a safe location. Then, set the resource
 * counter to the effective number of consumers. Now, check if it is
 * safe to return. This means that the location pointed, if resource
 * counter index is updated, should be a valid location. Valid location
 * means, data can be safely overwritten which means, resource counter
 * for that location should be zero. Until it is so, get into a loop.
 * Within the loop, if it is not safe, we brodcast on the conditional
 * variable associated with the queue and wait on this C.V.. If it is 
 * safe, we update the resource counter index, on the C.V., unlock the
 * mutex, and return safely.
 *
 * The conditional variable is used to signal buffer change (input and
 * output) between all the sharing threads.
 */
int __bc_internal_farmn_q_put(bc_farmn_q_t *q, int c) {
    int i;

    bc_mutex_lock(&(q->lock));

    /* Update variable pointer. */
    q->pptr.var += q->size;
    if (q->pptr.var == q->end)
        q->pptr.var = q->start;

    /* Set reference counter. */
    q->rc[q->pptr.rcidx] = 1;

    /* Check if it is safe to return. */
    i = q->pptr.rcidx + 1;
    i %= q->du;
    while (1) {
        if (q->rc[i] == 0) {
            q->pptr.rcidx = i;
            bc_cond_broadcast(&(q->cond));
            bc_mutex_unlock(&(q->lock));
            return 0;
        }
        bc_cond_broadcast(&(q->cond));
        bc_cond_wait(&(q->cond), &(q->lock));
    }
    return 0;
}

/*
 * Get:
 *
 * First lock the queue. Check if data is ready for *ME*. If data is 
 * ready, the rc at rc index should be > 0, and this rc index should 
 * not equal the one associated with the producer's rc index. This is
 * to avoid the same thread removing data from the same location more
 * than once. If these condition are not satisfied, we wait on the
 * C.V. until the buffer gets updated.
 *
 * If it is safe to access the buffer, we send the data to the remote
 * computation thread. Then, we update the variable pointer, and update
 * rc and rc index. We brodcast on the C.V. to signal buffer change.
 * Finally, the mutex is unlocked and function returns safely.
 */
int __bc_internal_farmn_q_send(bc_farmn_q_t *q, int fd, bc_vptr_t *vptr,
        int cond_recv) {
    bc_mutex_lock(&(q->lock));

    /* Is data ready? */
    while (1) {
        if (q->rc[q->cptr.rcidx] && (q->pptr.rcidx != q->cptr.rcidx))
            break;
        if (cond_recv) {
            __bc_send_sorry_message(fd);
            bc_mutex_unlock(&(q->lock));
            return 0;
        } else
            bc_cond_wait(&(q->cond), &(q->lock));
    }

    /* Send the data. */
    if (cond_recv) __bc_send_preamble_message(fd);
    __bc_write_data(fd, q->cptr.var, q->size);

    /* Update link variable pointer. */
    q->cptr.var += q->size;
    if (q->cptr.var == q->end)
        q->cptr.var = q->start;

    /* Update rc and rc index. */
    q->rc[q->cptr.rcidx]--;
    q->cptr.rcidx++;
    q->cptr.rcidx %= q->du;

    /* Signal queue change. */
    bc_cond_broadcast(&(q->cond));
    bc_mutex_unlock(&(q->lock));

    return 0;
}

