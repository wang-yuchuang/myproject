/**
 * @file common.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv animation application common interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef __SD_ANIMATION_COMMON_H__
#define __SD_ANIMATION_COMMON_H__
#include <driver/taishan/dispss/disp_data_type.h>

#define MAX_FRAMEBUFFER_NUM 10
#define MAX_DISPLAY_NUM 10
//#define DEBUG_TIME_PRINT
#define BOOTANIM_USE_IRAM

/* ANIM type */
typedef enum anim_type_e{
    E_ANIM_TYPE_STATIC_LOGO,
    E_ANIM_TYPE_DYNAMIC_MJPEG,
    E_ANIM_TYPE_MAX
} ANIM_TYPE_E;

/* ANIM states */
typedef enum anim_state_e{
    E_ANIM_STATE_NONE,
    E_ANIM_STATE_PLAYING,
    E_ANIM_STATE_STOP,
    E_ANIM_STATE_MAX
} ANIM_STATE_E;

/* ANIM display info */
typedef struct anim_display_info_t{
    uint32_t dw;
    uint32_t dh;
    uint32_t aw;
    uint32_t ah;
    bool show_anim;
} ANIM_DISPLAY_INFO_T;

/* ANIM buffer info */
typedef struct anim_buffer_info_t{
    uint32_t w;
    uint32_t h;
    uint32_t pix_bytes;
    uint32_t stride[3];
    uint32_t buf_size[3];
    DISP_COLOR_ENUM format;
    uint32_t fb_num;
    uint32_t fb_decoder;
    uint32_t fb_render;
    uint8_t *fb_addr[MAX_FRAMEBUFFER_NUM];

} ANIM_BUFFER_INFO_T;

/* ANIM buffer info */
typedef struct anim_config_info_t{
    ANIM_TYPE_E type;
    uint32_t display_num;
    ANIM_DISPLAY_INFO_T display_info[MAX_DISPLAY_NUM]; //max 10
} ANIM_CONFIG_INFO_T;

/* ANIM buffer info */
typedef struct anim_data_t {
    ANIM_CONFIG_INFO_T *config_info;
    ANIM_BUFFER_INFO_T *buf_info;
    ANIM_STATE_E state;
} ANIM_DATA_T;

struct anim_info_t;
typedef int (*FUNCTION)(struct anim_info_t *ptr); //ANIM_INFO_T *

/* ANIM component info */
typedef struct anim_component_t {
     FUNCTION c_init;
     FUNCTION c_work;
     FUNCTION c_deinit;
} ANIM_COMPONENT_T;

/* ANIM all info */
typedef struct anim_info_t {
     ANIM_COMPONENT_T *a_player;
     ANIM_COMPONENT_T *a_decoder;
     ANIM_COMPONENT_T *a_render;
     ANIM_DATA_T    *a_data;
} ANIM_INFO_T;

#endif /*__SD_ANIMATION_COMMON_H__*/