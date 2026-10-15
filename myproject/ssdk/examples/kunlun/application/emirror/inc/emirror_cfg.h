/**
 * @file emirror_cfg.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv animation application.
 *
 * Revision History:
 * -----------------
 */
#ifndef __SD_EMIRROR_CFG_H__
#define __SD_EMIRROR_CFG_H__

#define IMG_HEIGHT  720
#define IMG_WIDTH   1280
#define CAM_HEIGHT  720
#define CAM_WIDTH   1280
#define AVM_HEIGHT  800
#define AVM_WIDTH   640
#define IMG_COUNT   4
#define CAM_MAX_BUF 3

#define LEFT_MIRROR  CONTROLPANEL
#define RIGHT_MIRROR HUD

enum {
    EMIRROR_CSI_TASK = 0,
    EMIRROR_PLAYER_0_TASK,
    EMIRROR_PLAYER_1_TASK,
#ifdef CONFIG_USE_GUI_LVGL
    EMIRROR_LVGL_TASK,
#endif
    EMIRROR_TASK_CNT,
};

enum {
    CAM_LEFT = 0,
    CAM_RIGHT,
    CAM_NUM
};

#define EMIRROR_MASK(i) (1<<(i))

#ifndef u8
#define u8 unsigned char
#endif
#ifndef u16
#define u16 unsigned short
#endif
#ifndef u32
#define u32 unsigned int
#endif
#ifndef u64
#define u64 unsigned long long
#endif

#endif /* __SD_EMIRROR_CFG_H__ */
/* EOF */
