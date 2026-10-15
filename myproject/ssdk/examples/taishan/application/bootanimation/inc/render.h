/**
 * @file decoder.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv animation application render interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef __SD_RENDER_H__
#define __SD_RENDER_H__
#include <cmsis_os2.h>
#include <board.h>
#include <debug.h>
#include "common.h"

int render_init(ANIM_INFO_T *anim_info_ptr);
int render_work(ANIM_INFO_T *anim_info_ptr);
int render_deinit(ANIM_INFO_T *anim_info_ptr);

#endif /*__SD_RENDER_H__*/