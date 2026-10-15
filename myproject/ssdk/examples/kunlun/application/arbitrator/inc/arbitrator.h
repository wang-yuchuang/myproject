/**
 * @file arbitrator.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv arbitrator library.
 *
 * Revision History:
 * -----------------
 */
#ifndef __SD_ARBITRATOR_H__
#define __SD_ARBITRATOR_H__
#include <lib/list.h>

typedef struct _arb_accessor_t arb_accessor_t;
typedef struct _arb_ops_t arb_ops_t;
typedef void* arb_slave_t;
typedef struct _arb_master_t arb_master_t;
typedef struct _arb_ctx_t arb_ctx_t;

typedef enum _arb_magic_em {
    arb_magic_video = 0,
    arb_magic_max
} arb_magic_em;

typedef enum _arb_prio_em {
    arb_prio_low = 0,
    arb_prio_mid,
    arb_prio_high,
} arb_prio_em;

typedef enum _arb_preempt_em {
    arb_preempt_never = 0,
    arb_preempt_always,
    arb_preempt_bfap,
    arb_preempt_afap,
} arb_preempt_em;

struct _arb_accessor_t {
    const int magic;
    const int slave_tk;
    int token;
    int prio;
    struct list_node node;
    void (*p_start)(int magic, int token, void* accessor);
    void (*p_stop)(int magic, int token, void* accessor);
    void (*p_suspend)(int magic, int token, void* accessor);
    void (*p_resume)(int magic, int token, void* accessor);
};

struct _arb_ops_t {
    void (*p_init)(arb_master_t* master);
    void (*p_request)(arb_master_t* master, int token, arb_accessor_t* accessor);
    void (*p_release)(arb_master_t* master, int token, arb_accessor_t* accessor);
    void (*p_getHandle)(arb_master_t* master, int token, void** handle);
    void (*p_preemptChange)(arb_master_t* master, int token, int preempt);
    void (*p_update)(arb_master_t* master, int token);
    void (*p_query)(arb_master_t* master, int token, int* accessor_tk);
};

struct _arb_master_t {
    const int magic;
    int slave_num;
    void* slave;
    arb_ops_t* ops;
};

struct _arb_ctx_t {
    int init;
    int accessor_cnt;
    osMutexId_t accessor_mtx;
    int ap_running;
    const int num;
    arb_master_t* master;
};

extern int arb_init();
extern int arb_request_gain_slave(int magic, int token, arb_accessor_t* accessor);
extern int arb_request_release_slave(int magic, int token, arb_accessor_t* accessor);
extern int arb_accessor_attach(arb_accessor_t* accessor);
extern int arb_query_slave_peak_accessor(int magic, int token);
extern void* arb_get_slave_handle(int magic, int token);
extern int arb_preempt_change(int magic, int token, int preempt);
extern int arb_is_ap_running();
#endif /* __SD_ARBITRATOR_H__ */
/* EOF */
