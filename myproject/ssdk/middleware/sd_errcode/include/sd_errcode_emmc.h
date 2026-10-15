/*
 * sd_errcode_emmc.h
 *
 * Copyright (c) 2022 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 * errcode to eMMC head file
 *
 * Revision History:
 * -----------------
 */

#ifndef _SD_ERRCODE_EMMC_H_
#define _SD_ERRCODE_EMMC_H_

#include <sys/types.h>
#include "errcode.h"

#define CONFIG_ITEM_NAME_LEN     16
#define CONFIG_ITEM_NUM_MAX      512  /* mod_id*mode_no*errcode 15*2*30 = 900*/

/** errcode item management info  */
typedef struct {
    uint32_t id;
    uint8_t  name[CONFIG_ITEM_NAME_LEN];
}errcode_item_t;

typedef struct {
    errcode_item_t item[CONFIG_ITEM_NUM_MAX];
    uint32_t item_num;
}errcode_item_info_t;

int errcode_emmc_init(void);
int errcode_emmc_deinit(void);
ERR_HANDLE errcode_set_to_emmc(int strid, uint8_t *data_buffer, uint16_t length);
int errcode_dump_from_emmc(int strid, char* buf, int bufsize);
int errcode_clear_emmc(int strid);
#endif //_SD_ERRCODE_EMMC_H_
