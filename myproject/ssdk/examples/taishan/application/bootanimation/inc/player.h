/**
 * @file decoder.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv animation application player interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef __SD_PLAYER_H__
#define __SD_PLAYER_H__
#include <cmsis_os2.h>
#include <board.h>
#include <debug.h>
#include "common.h"

int player_init(ANIM_INFO_T *anim_info_ptr);
int player_work(ANIM_INFO_T *anim_info_ptr);
int player_deinit(ANIM_INFO_T *anim_info_ptr);
#endif /* __SD_PLAYER_H__ */