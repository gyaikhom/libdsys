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
#error "Please do not include 'threads.h'; include 'dsys.h' instead."
#endif

#ifndef _BC_THREADS_H
#define _BC_THREADS_H

#include "common.h"

#if HAVE_THREAD_H /* Solaris Thread */
#include <thread.h>
#elif HAVE_PTHREAD_H /* POSIX Thread */
#include <pthread.h>
#endif

BEGIN_C_DECLS

#if HAVE_THREAD_H /* Solaris Thread */
typedef thread_t bc_thread_t;
typedef mutex_t bc_mutex_t;
typedef cond_t bc_cond_t;
#elif HAVE_PTHREAD_H /* POSIX Thread */
typedef pthread_t bc_thread_t;
typedef pthread_mutex_t bc_mutex_t;
typedef pthread_cond_t bc_cond_t;
#endif

typedef struct bc_tmgr_s {
    unsigned int nst; /* Number of service threads */
    bc_mutex_t lock; /* Lock */
    bc_cond_t cond; /* Conditional variable */
    bc_thread_t mgr; /* Manager thread */
} bc_tmgr_t;

int __bc_sthreads_create(void);

/* Define frequent functions as macros. */
#if HAVE_THREAD_H /* Solaris Thread */

#define bc_thread_create(thread, sf, arg) \
        thr_create(NULL, 0, sf, arg, NULL, thread)
#define bc_mutex_init(mx) \
        mutex_init(mx, NULL, NULL)
#define bc_mutex_lock(mx) \
        mutex_lock(mx)
#define bc_mutex_unlock(mx) \
        mutex_unlock(mx)
#define bc_mutex_destroy(mx) \
        mutex_destroy(mx)
#define bc_cond_init(cv) \
        cond_init(cv, NULL, NULL)
#define bc_cond_wait(cv, mx) \
        cond_wait(cv, mx)
#define bc_cond_twait(cv, mx, abstime) \
        cond_timedwait(cv, mx, abstime)
#define bc_cond_signal(cv) \
        cond_signal(cv)
#define bc_cond_broadcast(cv) \
        cond_broadcast(cv)
#define bc_cond_destroy(cv) \
        cond_destroy(cv)
#define bc_join(thread, status) \
        thr_join(thread, NULL, status)
#define bc_thread_exit(rval) \
        thr_exit(rval)

#elif HAVE_PTHREAD_H    /* POSIX Thread */

#define bc_thread_create(thread, sf, arg) \
        pthread_create(thread, NULL, sf, arg)
#define bc_mutex_init(mx) \
        pthread_mutex_init(mx, NULL)
#define bc_mutex_lock(mx) \
        pthread_mutex_lock(mx)
#define bc_mutex_unlock(mx) \
        pthread_mutex_unlock(mx)
#define bc_mutex_destroy(mx) \
        pthread_mutex_destroy(mx)
#define bc_cond_init(cv) \
        pthread_cond_init(cv, NULL)
#define bc_cond_wait(cv, mx) \
        pthread_cond_wait(cv, mx)
#define bc_cond_twait(cv, mx, abstime) \
        pthread_cond_timedwait(cv, mx, abstime)
#define bc_cond_signal(cv) \
        pthread_cond_signal(cv)
#define bc_cond_broadcast(cv) \
        pthread_cond_broadcast(cv)
#define bc_cond_destroy(cv) \
        pthread_cond_destroy(cv)
#define bc_join(thread, status) \
        pthread_join(thread, status)
#define bc_thread_exit(rval) \
        pthread_exit(rval);

#endif

END_C_DECLS

#endif /* _BC_THREADS_H */
