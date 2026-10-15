/**
 * @file ani_def.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv boot animation application.
 *
 * Revision History:
 * -----------------
 */

#ifndef __SD_ANI_DEF_H__
#define __SD_ANI_DEF_H__
#include <cmsis_os2.h>
#include <sdm_panel.h>
#include "ani_cfg.h"
#include "container.h"

typedef enum _ani_st_em {
    ani_st_invalid = -1,
    ani_st_uninit = 0,
    ani_st_idle,
    ani_st_run,
    ani_st_max
} ani_st_em;

typedef enum _ani_evt_em {
    ani_evt_invalid = -1,
    ani_evt_init = 0,
    ani_evt_playback,
    ani_evt_hide,
    ani_evt_show,
    ani_evt_stop,
    ani_evt_elapsed, /* 5 */
    ani_evt_max
} ani_evt_em;

typedef enum _ani_evt_param_em {
    ani_evt_param_openning = 0,
    ani_evt_param_replay = 1,
} ani_evt_param_em;

typedef struct _ani_task_token_t {
    int id;
    const char* name;
    int running;
    osThreadId_t thread_id;
    osThreadAttr_t thread_attr;
    void (*thread_task)(void* args);
    int disp_sts;
    const int support_disp;
    const int support_res;
    osEventFlagsId_t terminate_sig;
    const char* issue_magic;
    int issue_num;
    unsigned long issue_size;
    issue_token_t* issue;
    osMutexId_t disp_mtx;
} ani_task_token_t;

typedef struct _ani_ctx_t {
    int init;   /* init once */
    int* property_id;
    osMessageQueueId_t launcher_q; /* sender */
    msg_token_t* stm_msg;
    ani_task_token_t* task_tk;
    uint32_t detect_task;
    osEventFlagsId_t task_terminate;
} ani_ctx_t;

typedef struct _ani_mjpeg_t {
    const char* path;
    uint32_t size;
    uint32_t offset;
} ani_mjpeg_t;

typedef struct _ani_frame_ele_t {
    int disp_id;
    int res_id;
    rect_t  src_rect;
    rect_t  dest_rect;
} ani_frame_ele_t;

#define ANI_DISP_BUSY(task) \
        ({ \
            osStatus_t ret = osMutexAcquire((task)->disp_mtx, osWaitForever); \
            ret; \
        })

#define ANI_DISP_IDLE(task, ret) \
        if ((ret) == osOK) { \
            osMutexRelease((task)->disp_mtx); \
        }

#endif /* __SD_ANI_DEF_H__ */
/* EOF */
