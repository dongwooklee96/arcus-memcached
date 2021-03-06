/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * arcus-memcached - Arcus memory cache server
 * Copyright 2010-2014 NAVER Corp.
 * Copyright 2014-2020 JaM2in Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef THREAD_H
#define THREAD_H

#include <event.h>
#include "cache.h"
#include "topkeys.h"
#include "mc_util.h"

#define LOCK_THREAD(t)                          \
    if (pthread_mutex_lock(&t->mutex) != 0) {   \
        abort();                                \
    }                                           \
    assert(t->is_locked == false);              \
    t->is_locked = true;

#define UNLOCK_THREAD(t)                         \
    assert(t->is_locked == true);                \
    t->is_locked = false;                        \
    if (pthread_mutex_unlock(&t->mutex) != 0) {  \
        abort();                                 \
    }

/** Stats stored per slab (and per thread). */
struct slab_stats {
    uint64_t  cmd_set;
    uint64_t  get_hits;
    uint64_t  delete_hits;
    uint64_t  cas_hits;
    uint64_t  cas_badval;
};

/**
 * Stats stored per-thread.
 */
struct thread_stats {
    pthread_mutex_t   mutex;
    uint64_t          cmd_get;
    uint64_t          cmd_incr;
    uint64_t          cmd_decr;
    uint64_t          cmd_delete;
    uint64_t          get_misses;
    uint64_t          delete_misses;
    uint64_t          incr_misses;
    uint64_t          decr_misses;
    uint64_t          incr_hits;
    uint64_t          decr_hits;
    uint64_t          cmd_cas;
    uint64_t          cas_misses;
    uint64_t          bytes_read;
    uint64_t          bytes_written;
    uint64_t          cmd_flush;
    uint64_t          cmd_flush_prefix;
    uint64_t          conn_yields; /* # of yields for connections (-R option)*/
    uint64_t          auth_cmds;
    uint64_t          auth_errors;
    /* list command stats */
    uint64_t          cmd_lop_create;
    uint64_t          cmd_lop_insert;
    uint64_t          cmd_lop_delete;
    uint64_t          cmd_lop_get;
    /* set command stats */
    uint64_t          cmd_sop_create;
    uint64_t          cmd_sop_insert;
    uint64_t          cmd_sop_delete;
    uint64_t          cmd_sop_get;
    uint64_t          cmd_sop_exist;
    /* map command stats */
    uint64_t          cmd_mop_create;
    uint64_t          cmd_mop_insert;
    uint64_t          cmd_mop_update;
    uint64_t          cmd_mop_delete;
    uint64_t          cmd_mop_get;
    /* btree command stats */
    uint64_t          cmd_bop_create;
    uint64_t          cmd_bop_insert;
    uint64_t          cmd_bop_update;
    uint64_t          cmd_bop_delete;
    uint64_t          cmd_bop_get;
    uint64_t          cmd_bop_count;
    uint64_t          cmd_bop_position;
    uint64_t          cmd_bop_pwg;
    uint64_t          cmd_bop_gbp;
#ifdef SUPPORT_BOP_MGET
    uint64_t          cmd_bop_mget;
#endif
#ifdef SUPPORT_BOP_SMGET
    uint64_t          cmd_bop_smget;
#endif
    uint64_t          cmd_bop_incr;
    uint64_t          cmd_bop_decr;
    /* attr command stats */
    uint64_t          cmd_getattr;
    uint64_t          cmd_setattr;
    /* list hit & miss stats */
    uint64_t          lop_create_oks;
    uint64_t          lop_insert_hits;
    uint64_t          lop_insert_misses;
    uint64_t          lop_delete_elem_hits;
    uint64_t          lop_delete_none_hits;
    uint64_t          lop_delete_misses;
    uint64_t          lop_get_elem_hits;
    uint64_t          lop_get_none_hits;
    uint64_t          lop_get_misses;
    /* set hit & miss stats */
    uint64_t          sop_create_oks;
    uint64_t          sop_insert_hits;
    uint64_t          sop_insert_misses;
    uint64_t          sop_delete_elem_hits;
    uint64_t          sop_delete_none_hits;
    uint64_t          sop_delete_misses;
    uint64_t          sop_get_elem_hits;
    uint64_t          sop_get_none_hits;
    uint64_t          sop_get_misses;
    uint64_t          sop_exist_hits;
    uint64_t          sop_exist_misses;
    /* map hit & miss stats */
    uint64_t          mop_create_oks;
    uint64_t          mop_insert_hits;
    uint64_t          mop_insert_misses;
    uint64_t          mop_update_elem_hits;
    uint64_t          mop_update_none_hits;
    uint64_t          mop_update_misses;
    uint64_t          mop_delete_elem_hits;
    uint64_t          mop_delete_none_hits;
    uint64_t          mop_delete_misses;
    uint64_t          mop_get_elem_hits;
    uint64_t          mop_get_none_hits;
    uint64_t          mop_get_misses;
    /* btree hit & miss stats */
    uint64_t          bop_create_oks;
    uint64_t          bop_insert_hits;
    uint64_t          bop_insert_misses;
    uint64_t          bop_update_elem_hits;
    uint64_t          bop_update_none_hits;
    uint64_t          bop_update_misses;
    uint64_t          bop_delete_elem_hits;
    uint64_t          bop_delete_none_hits;
    uint64_t          bop_delete_misses;
    uint64_t          bop_get_elem_hits;
    uint64_t          bop_get_none_hits;
    uint64_t          bop_get_misses;
    uint64_t          bop_count_hits;
    uint64_t          bop_count_misses;
    uint64_t          bop_position_elem_hits;
    uint64_t          bop_position_none_hits;
    uint64_t          bop_position_misses;
    uint64_t          bop_pwg_elem_hits;
    uint64_t          bop_pwg_none_hits;
    uint64_t          bop_pwg_misses;
    uint64_t          bop_gbp_elem_hits;
    uint64_t          bop_gbp_none_hits;
    uint64_t          bop_gbp_misses;
#ifdef SUPPORT_BOP_MGET
    uint64_t          bop_mget_oks;
#endif
#ifdef SUPPORT_BOP_SMGET
    uint64_t          bop_smget_oks;
#endif
    uint64_t          bop_incr_elem_hits;
    uint64_t          bop_incr_none_hits;
    uint64_t          bop_incr_misses;
    uint64_t          bop_decr_elem_hits;
    uint64_t          bop_decr_none_hits;
    uint64_t          bop_decr_misses;
    /* attr hit & miss stats */
    uint64_t          getattr_hits;
    uint64_t          getattr_misses;
    uint64_t          setattr_hits;
    uint64_t          setattr_misses;
    struct slab_stats slab_stats[MAX_SLAB_CLASSES];
};

/**
 * The stats structure the engine keeps track of
 */
struct independent_stats {
    topkeys_t *topkeys;
    struct thread_stats thread_stats[];
};

enum thread_type {
    GENERAL = 11
};

typedef struct {
    pthread_t thread_id;        /* unique ID of this thread */
    struct event_base *base;    /* libevent handle this thread uses */
    struct event notify_event;  /* listen event for notify pipe */
    int notify_receive_fd;      /* receiving end of notify pipe */
    int notify_send_fd;         /* sending end of notify pipe */
    struct conn_queue *new_conn_queue; /* queue of new connections to handle */
    cache_t *suffix_cache;      /* suffix cache */
    pthread_mutex_t mutex;      /* Mutex to lock protect access to the pending_io */
    bool is_locked;
    struct conn *pending_io;           /* List of connection with pending async io ops */
    struct conn *conn_list;            /* connection list managed by this thread */
    int index;                  /* index of this thread in the threads array */
    enum thread_type type;      /* Type of IO this thread processes */
    token_buff_t token_buff;    /* token buffer */
    mblck_pool_t mblck_pool;    /* memory block pool */
} LIBEVENT_THREAD;

// Number of times this connection is in the given pending list
int    number_of_pending(struct conn *c, struct conn *pending);
bool   has_cycle(struct conn *c);
bool   list_contains(struct conn *h, struct conn *n);
struct conn  *list_remove(struct conn *h, struct conn *n);
size_t list_to_array(struct conn **dest, size_t max_items, struct conn **l);

void notify_io_complete(const void *cookie, ENGINE_ERROR_CODE status);
int  dispatch_event_add(int thread, struct conn *c);
void dispatch_conn_new(int sfd, STATE_FUNC init_state, int event_flags,
                       int read_buffer_size, enum network_transport transport);
int  is_listen_thread(void);

void threadlocal_stats_clear(struct thread_stats *stats);
void threadlocal_stats_reset(struct thread_stats *thread_stats);
void threadlocal_stats_aggregate(struct thread_stats *thread_stats, struct thread_stats *stats);
void slab_stats_aggregate(struct thread_stats *stats, struct slab_stats *out);

void thread_init(int nthreads, struct event_base *main_base);
void threads_shutdown(void);
#endif
