/**
 * @file emirror_def.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv animation application.
 *
 * Revision History:
 * -----------------
 */
#ifndef __SD_EMIRROR_DEF_H__
#define __SD_EMIRROR_DEF_H__
#include <stdint.h>
#include <cmsis_os2.h>
#include <vdev_defs.h>
#include <sdm_display.h>
#include "container.h"
#include "emirror_cfg.h"

typedef enum _emirror_st_em {
    emirror_st_invalid = -1,
    emirror_st_uninit = 0,
    emirror_st_idle,
    emirror_st_run,
    emirror_st_max
} emirror_st_em;

typedef enum _emirror_evt_em {
    emirror_evt_invalid = -1,
    emirror_evt_init = 0,
    emirror_evt_start,
    emirror_evt_stop,
    emirror_evt_max
} emirror_evt_em;

typedef enum _emirror_sig_em {
    emirror_sig_invalid = 0,
    emirror_sig_hand,
    emirror_sig_take,
    emirror_sig_max
} emirror_sig_em;

typedef struct _emirror_task_token_t {
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
} emirror_task_token_t;

typedef struct _emirror_ctx_t {
    int init;   /* init once */
    const int* property_id;
    osMessageQueueId_t launcher_q; /* sender */
    msg_token_t* stm_msg;
    emirror_task_token_t* task_tk;
    uint32_t detect_task;
    osEventFlagsId_t task_terminate;
} emirror_ctx_t;

typedef struct _emirror_vstream_t {
    sdm_display_t* disp_handle;
    unsigned long addr;
    uint32_t cam_w, cam_h;
    uint32_t stride;
} emirror_vstream_t;

typedef struct _emi_csi_buff_t {
    uint32_t planes;
    uint32_t pix_bytes;
    uint32_t w;
    uint32_t h;
    uint32_t fmt;
    uint32_t fps;
    uint32_t mem_size[3];
    void* mem_ptr[3];
    uint32_t stride[3];
    uint32_t buf_size[3];
    void* buffers[CAM_MAX_BUF][3];
} emi_csi_buff_t;

typedef struct _emirror_csi_ops_t {
    int (*p_init)(int id, uint32_t* w, uint32_t* h, uint32_t* fmt, uint32_t* f);
    int (*p_deinit)(int id);
    int (*p_config)(int id, emi_csi_buff_t* buff, uint32_t cnt);
    int (*p_setbuf)(int id, struct vdev_buffer* buf);
    int (*p_getbuf)(int id, struct vdev_buffer* buf);
    int (*p_start)(int id);
    int (*p_stop)(int id);
    int (*p_alloc)(int id, emi_csi_buff_t* buff);
} emirror_csi_ops_t;

typedef struct _emirror_g2dlite_ops_t {
    int (*p_inputInit)(void* buf, uint32_t w, uint32_t h);
} emirror_g2dlite_ops_t;

typedef struct _emirror_vdsp_ops_t {
    int (*p_stall)();
    int (*p_reset)();
    int (*p_load_map_tbl)(void** vdsp_map_tbl);
    int (*p_load_firmware)();
    int (*p_go)();
} emirror_vdsp_ops_t;

typedef struct _emirror_rect_t {
    uint32_t x;
    uint32_t y;
    uint32_t w;
    uint32_t h;
} emirror_rect_t;

extern void emirror_camera_get_roi(int cam, emirror_rect_t* rect);
extern void emirror_camera_set_roi(int cam, emirror_rect_t* rect);
extern void emirror_camera_adj_roi(int cam, int direction);

#endif /* __SD_EMIRROR_DEF_H__ */
/* EOF */
