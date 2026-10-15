/**
 * @file am_ctl.h
 * @brief audiomanager control header file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef __AM_CTL_H__
#define __AM_CTL_H__

#include <stdio.h>

#include "am_codec.h"

bool am_reset_caches(am_codec_t *codec);
bool am_burn_fw(am_codec_t *codec, int fw_no);
bool am_set_cofficient(am_codec_t *codec, int mode, int param_size,
                       unsigned char *param);
bool am_sync_caches(am_codec_t *codec, am_reg_t *am_regs);
bool am_read_ctl(am_codec_t *codec, unsigned int ctl_name, unsigned int *val);
bool am_write_ctl(am_codec_t *codec, unsigned int ctl_name, unsigned int val);
bool am_write_ctl_nocache(am_codec_t *codec, unsigned int ctl_name,
                          unsigned int val);

bool am_read_reg(am_codec_t *codec, unsigned int reg_addr, unsigned int *val);
bool am_write_reg(am_codec_t *codec, unsigned int reg_addr, unsigned int val);
bool am_write_reg_nocache(am_codec_t *codec, unsigned int reg_addr,
                          unsigned int val);
bool am_make_dirty_ctl(am_codec_t *codec, unsigned int ctl_name);
bool am_make_dirty_codec(am_codec_t *codec);
bool am_make_dirty_reg(am_codec_t *codec, unsigned int reg_addr);
bool am_dump_regs(am_codec_t *codec);

#endif
