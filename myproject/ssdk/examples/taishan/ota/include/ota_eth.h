/**
 * @file ota_eth.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ota from ethernet
 *
 * Revision History:
 * -----------------
 */


#ifndef  _OTA_ETH_
#define  _OTA_ETH_

extern uint8_t ota_tftp_init_client(void *handle);
extern void *create_tftp_handle(DISK_STATE_T *ds, const char *fname,
                                uint8_t max_bpt_num, uint8_t min_bpt_num, uint8_t inst_num);
#endif
