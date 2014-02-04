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

#ifndef __BC_SYS_COMPILE
#error "Please do not include 'queue.h'; include 'dsys.h' instead."
#endif

#ifndef _BC_QUEUE_H
#define _BC_QUEUE_H

#include "common.h"
#include "plist.h"

BEGIN_C_DECLS

typedef enum {
    __BC_QTYPE_CUST = 0,
    __BC_QTYPE_MC121, /* 1 to 1 with memory copy. */
    __BC_QTYPE_NMC121, /* 1 to 1 without memory copy. */
    __BC_QTYPE_MC12M, /* 1 to many with memory copy. */
    __BC_QTYPE_NMC12M, /* 1 to many without memory copy. */
    __BC_QTYPE_FARMN, /* Fan out buffer. */

    __BC_QTYPE_NUM
} bc_qtype_t;

typedef struct bc_mc121_s bc_mc121_t;

struct bc_mc121_s {
    bc_mutex_t lock; /* Queue lock */
    size_t bytes_inbuff; /* Data bytes in buff */
    size_t size; /* Size of the queue (in bytes) */
    void *free_start; /* Point where free memory start */
    void *data_start; /* Point where data start */
    void *buff_start; /* Queue buffer start */
    void *buff_end; /* End of queue buffer */
    bc_cond_t no_suff; /* C.V. for not suff. space/data */
};

typedef struct bc_mc12m_s bc_mc12m_t;

struct bc_mc12m_s {
    unsigned int tag;
    bc_mc121_t *mc121;
    bc_mc12m_t *next;
};

typedef struct bc_vptr_s bc_vptr_t;

struct bc_vptr_s {
    void *var;
    int rcidx;
};

typedef struct bc_nmc121_s bc_nmc121_t;

struct bc_nmc121_s {
    void *start;
    void *end;
    size_t size;
    int inbuff;
    int du;
    bc_vptr_t vptr;
    bc_cond_t cond;
    bc_mutex_t lock;
};

typedef struct bc_nmc12m_s bc_nmc12m_t;

struct bc_nmc12m_s {
    unsigned short *rc;
    void *start;
    void *end;
    size_t size;
    int du;
    bc_vptr_t vptr;
    bc_cond_t cond;
    bc_mutex_t lock;
};

typedef struct bc_farmn_q_s bc_farmn_q_t;

struct bc_farmn_q_s {
    unsigned short *rc;
    void *start;
    void *end;
    size_t size;
    int du;
    bc_vptr_t pptr;
    bc_vptr_t cptr;
    bc_cond_t cond;
    bc_mutex_t lock;
};

typedef union bc_queue_s {
    bc_mc121_t *mc121;
    bc_mc12m_t *mc12m;
    bc_nmc121_t *nmc121;
    bc_nmc12m_t *nmc12m;
    bc_farmn_q_t *nmcus;
} bc_queue_t;

typedef bc_queue_t *(*bc_qc_t) (bc_plist_t *pl, int du, size_t dusz);
typedef int (*bc_qd_t) (bc_queue_t *q);
typedef int (*bc_qp_t) (bc_queue_t *q, void *local, size_t bytes, int tag);
typedef int (*bc_qs_t) (bc_queue_t *q, int fd, size_t bytes, int tag,
        int cond_recv);
typedef int (*bc_qsn_t) (bc_queue_t *q, int fd, size_t bytes, bc_vptr_t *ptr,
        int cond_recv);

typedef struct {
    bc_qc_t c; /* Create queue. */
    bc_qd_t d; /* Destroy queue. */
    bc_qp_t p; /* Put into queue. */
    bc_qs_t s; /* Send from queue. */
    bc_qsn_t sn; /* Send from queue (no memory copy). */
} bc_qiface_t;

extern bc_qiface_t bc_qiface[];

extern bc_mc121_t *__bc_mc121_create(bc_plist_t *pl, size_t bytes);
extern int __bc_mc121_destroy(bc_mc121_t *q);
extern int __bc_mc121_put(bc_mc121_t *q, void *local, size_t bytes, int tag);
extern int __bc_mc121_send(bc_mc121_t *q, int fd, size_t bytes, int tag,
        int cond_recv);
extern bc_queue_t *__bc_c_mc121(bc_plist_t *pl, int du, size_t szdu);
extern int __bc_d_mc121(bc_queue_t *q);
extern int __bc_p_mc121(bc_queue_t *q, void *local, size_t bytes, int tag);
extern int __bc_s_mc121(bc_queue_t *q, int fd, size_t bytes, int tag,
        int cond_recv);
extern int __bc_putq_mc121(bc_mc121_t *q, void *local, size_t bytes);
extern int __bc_sendq_mc121(bc_mc121_t *q, int fd, size_t bytes,
        int cond_recv);

extern bc_queue_t *__bc_c_mc12m(bc_plist_t *pl, int du, size_t bytes);
extern int __bc_d_mc12m(bc_queue_t *q);
extern int __bc_p_mc12m(bc_queue_t *q, void *local, size_t bytes, int tag);
extern int __bc_s_mc12m(bc_queue_t *q, int fd, size_t bytes, int tag,
        int cond_recv);

#define bc_var(bc, type) (*(((type *)(bc)->channel->q->nmc121->vptr.var)))
#define bc_vptr(bc, type) (((type *)(bc)->channel->q->nmc121->vptr.var))
extern bc_queue_t *__bc_c_nmc121(bc_plist_t *pl, int du, size_t szdu);
extern int __bc_d_nmc121(bc_queue_t *q);
extern int __bc_p_nmc121(bc_queue_t *q, void *local, size_t bytes, int tag);
extern int __bc_s_nmc121(bc_queue_t *q, int fd, size_t bytes, bc_vptr_t *vptr,
        int cond_recv);

extern bc_queue_t *__bc_c_nmc12m(bc_plist_t *pl, int du, size_t szdu);
extern int __bc_d_nmc12m(bc_queue_t *q);
extern int __bc_p_nmc12m(bc_queue_t *q, void *local, size_t bytes, int tag);
extern int __bc_s_nmc12m(bc_queue_t *q, int fd, size_t bytes, bc_vptr_t *vptr,
        int cond_recv);

extern bc_queue_t *__bc_c_farmn_q(bc_plist_t *pl, int du, size_t szdu);
extern int __bc_d_farmn_q(bc_queue_t *q);
extern int __bc_p_farmn_q(bc_queue_t *q, void *local, size_t bytes, int tag);
extern int __bc_s_farmn_q(bc_queue_t *q, int fd, size_t bytes,
        bc_vptr_t *vptr, int cond_recv);

END_C_DECLS

#endif /* _BC_QUEUE_H */
