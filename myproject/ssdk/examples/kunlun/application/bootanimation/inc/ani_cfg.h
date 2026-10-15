/**
 * @file ani_cfg.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv boot animation application.
 *
 * Revision History:
 * -----------------
 */

#ifndef __SD_ANI_CFG_H__
#define __SD_ANI_CFG_H__

enum {
    ANI_DEC_TASK = 0,
    ANI_PLAYER_TASK_0,
    //ANI_PLAYER_TASK_1,
    //ANI_PLAYER_TASK_2,
#if defined(CONFIG_AUDIO_MANAGER)
    ANI_AUDIO_TASK,
#endif
    ANI_RES_LOADER_TASK,
    ANI_TASK_CNT,
};

#define ANI_MASK(i)     (1<<(i))

#define ANI_RES_CNT     (1)
#define ANI_RES_PATH_BA "early_app/BootAnimation/splash_800x480_3s.mjpeg"
#define ANI_RES_PATH_BA_LOOP "early_app/BootAnimation/splash_800x480_1s.mjpeg"
#define ANI_RES_PATH_FA "early_app/BootAnimation/fast_audio_pcm.bin"
#define ANI_RES_PATH_TU "early_app/BootAnimation/turn_pcm.bin"
#define ANI_RES_PATH_WA "early_app/BootAnimation/warning_pcm.bin"

#define ANI_DISP_CNT            (1)  /* BMU05 single screen */
#define ANI_DISP_CLUSTER        ANI_MASK(CLUSTER)
#define ANI_DISP_INFOTAINMENT   ANI_MASK(INFOTAINMENT)
#define ANI_DISP_ENTERTAINMENT  ANI_MASK(ENTERTAINMENT)
#define ANI_DISP_CTRL_PANEL     ANI_MASK(CONTROLPANEL)
#define ANI_DISP_ALL_SUPPORT    (ANI_DISP_CLUSTER)  /* BMU05: only Cluster display */

#define ANI_FLAG_DEC_OFFSET     (0)
#define ANI_FLAG_DEC_MAX_OFFSET (7)
#define ANI_FLAG_PLAY_OFFSET    (8)
#define ANI_FLAG_PLAY_MAX_OFFSET (23)

#define ANI_FPS                 (25)

#endif /* __SD_ANI_CFG_H__ */
/* EOF */
