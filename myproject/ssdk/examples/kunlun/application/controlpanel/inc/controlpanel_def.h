/**
 * @file controlpanel_def.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv controlpanel application.
 *
 * Revision History:
 * -----------------
 */
#ifndef __SD_CONTROLPANEL_DEF_H__
#define __SD_CONTROLPANEL_DEF_H__
#include <stdint.h>
#include <cmsis_os2.h>
#include "container.h"

typedef enum _cp_st_em {
    cp_st_invalid = -1,
    cp_st_uninit = 0,
    cp_st_idle,
    cp_st_run,
    cp_st_suspend,
    cp_st_max
} cp_st_em;

typedef enum _cp_evt_em {
    cp_evt_invalid = -1,
    cp_evt_init = 0,
    cp_evt_start,
    cp_evt_stop,
    cp_evt_suspend,
    cp_evt_resume,
    cp_evt_max
} cp_evt_em;

typedef enum _cp_sig_em {
    cp_sig_invalid = 0,
    cp_sig_hand,
    cp_sig_take,
    cp_sig_max
} cp_sig_em;

typedef struct _cp_task_token_t {
    int id;
    const char* name;
    int running;
    osThreadId_t thread_id;
    osThreadAttr_t thread_attr;
    void (*thread_task)(void* args);
    int disp_sts;
    const int support_disp;
    osEventFlagsId_t terminate_sig;
} cp_task_token_t;

typedef struct _cp_ctx_t {
    int init;   /* init once */
    const int* property_id;
    osMessageQueueId_t launcher_q; /* sender */
    msg_token_t* stm_msg;
    cp_task_token_t* task_tk;
    uint32_t detect_task;
    osEventFlagsId_t task_terminate;
} cp_ctx_t;

#endif /* __SD_CONTROLPANEL_DEF_H__ */
/* EOF */
