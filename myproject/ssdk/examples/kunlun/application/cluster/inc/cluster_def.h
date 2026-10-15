/**
 * @file cluster_def.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv animation application.
 *
 * Revision History:
 * -----------------
 */
#ifndef __SD_CLUSTER_DEF_H__
#define __SD_CLUSTER_DEF_H__
#include <stdint.h>
#include <cmsis_os2.h>
#include "container.h"

typedef enum _cluster_st_em {
    cluster_st_invalid = -1,
    cluster_st_uninit = 0,
    cluster_st_idle,
    cluster_st_run,
    cluster_st_suspend,
    cluster_st_max
} cluster_st_em;

typedef enum _cluster_evt_em {
    cluster_evt_invalid = -1,
    cluster_evt_init = 0,
    cluster_evt_start,
    cluster_evt_stop,
    cluster_evt_suspend,
    cluster_evt_resume,
    cluster_evt_max
} cluster_evt_em;

typedef enum _cluster_sig_em {
    cluster_sig_invalid = 0,
    cluster_sig_hand,
    cluster_sig_take,
    cluster_sig_max
} cluster_sig_em;

typedef struct _cluster_task_token_t {
    int id;
    const char* name;
    int running;
    osThreadId_t thread_id;
    osThreadAttr_t thread_attr;
    void (*thread_task)(void* args);
    int disp_sts;
    const int support_disp;
    osEventFlagsId_t terminate_sig;
} cluster_task_token_t;

typedef struct _cluster_ctx_t {
    int init;   /* init once */
    const int* property_id;
    osMessageQueueId_t launcher_q; /* sender */
    msg_token_t* stm_msg;
    cluster_task_token_t* task_tk;
    uint32_t detect_task;
    osEventFlagsId_t task_terminate;
} cluster_ctx_t;

#endif /* __SD_CLUSTER_DEF_H__ */
/* EOF */
