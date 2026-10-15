/**
 * @file decoder.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv animation application decoder interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef __SD_DECODER_H__
#define __SD_DECODER_H__
#include <cmsis_os2.h>
#include <board.h>
#include <debug.h>
#include "common.h"

int decoder_init(ANIM_INFO_T *anim_info_ptr);
int decoder_work(ANIM_INFO_T *anim_info_ptr);
int decoder_deinit(ANIM_INFO_T *anim_info_ptr);

#endif /*__SD_DECODER_H__*/