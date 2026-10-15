/**
 * @file fastavm_def.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv animation application.
 *
 * Revision History:
 * -----------------
 */
#ifndef __SD_FASTAVM_DEF_H__
#define __SD_FASTAVM_DEF_H__
#include <stdint.h>
#include <cmsis_os2.h>
#include "container.h"

typedef enum _fastavm_st_em {
    fastavm_st_invalid = -1,
    fastavm_st_uninit = 0,
    fastavm_st_idle,
    fastavm_st_run,
    fastavm_st_max
} fastavm_st_em;

typedef enum _fastavm_evt_em {
    fastavm_evt_invalid = -1,
    fastavm_evt_init = 0,
    fastavm_evt_start,
    fastavm_evt_stop,
    fastavm_evt_max
} fastavm_evt_em;

typedef enum _fastavm_sig_em {
    fastavm_sig_invalid = 0,
    fastavm_sig_hand,
    fastavm_sig_take,
    fastavm_sig_max
} fastavm_sig_em;

typedef struct _fastavm_task_token_t {
    int id;
    const char* name;
    int running;
    osThreadId_t thread_id;
    osThreadAttr_t thread_attr;
    void (*thread_task)(void* args);
    int disp_sts;
    const int support_disp;
    osEventFlagsId_t terminate_sig;
    const char* issue_magic;
    int issue_num;
    unsigned long issue_size;
    issue_token_t* issue;
} fastavm_task_token_t;

typedef struct _fastavm_ctx_t {
    int init;   /* init once */
    const int* property_id;
    osMessageQueueId_t launcher_q; /* sender */
    msg_token_t* stm_msg;
    fastavm_task_token_t* task_tk;
    uint32_t detect_task;
    osEventFlagsId_t task_terminate;
} fastavm_ctx_t;

typedef struct _fastavm_vstream_t {
    unsigned long bufY;
    unsigned long bufCb;
    unsigned long bufCr;
    unsigned long strideY;
    unsigned long yuyv_addr;
    unsigned long yuyv_stride;
    uint32_t disp_w;
    uint32_t disp_h;
} fastavm_vstream_t;

typedef struct _fastavm_csi_ops_t {
    int (*p_open)(int id);
    int (*p_close)(int id);
    int (*p_init)(int id, uint32_t* w, uint32_t* h, uint32_t* fmt);
    int (*p_config)(int id, uint8_t *buf, uint32_t buf_size, uint32_t buf_cnt);
    int (*p_setbuf)(int id, uint32_t index);
    int (*p_getbuf)(int id, uint32_t *index, uint64_t *timestamp);
    int (*p_start)(int id);
    int (*p_stop)(int id);
} fastavm_csi_ops_t;

typedef struct _fastavm_g2dlite_ops_t {
    int (*p_inputInit)(void* buf, uint32_t w, uint32_t h);
} fastavm_g2dlite_ops_t;

typedef struct _fastavm_vdsp_ops_t {
    int (*p_stall)();
    int (*p_reset)();
    int (*p_load_map_tbl)(void** vdsp_map_tbl);
    int (*p_load_firmware)();
    int (*p_go)();
} fastavm_vdsp_ops_t;

#endif /* __SD_FASTAVM_DEF_H__ */
/* EOF */
