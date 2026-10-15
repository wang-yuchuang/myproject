/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef CMD_BASIC_MGMT_H
#define CMD_BASIC_MGMT_H

#include "cmd_common.h"

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[15];
    uint8_t pin_code[16];
} cmd_seip_init_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t info_len;
    uint8_t key_id[2];
    uint8_t device_info_ptr[4];
    uint8_t aut_info_ptr[4];
} cmd_get_device_info_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t status_len;
    uint8_t key_id[2];
    uint8_t device_status_ptr[4];
    uint8_t aut_info_ptr[4];
} cmd_get_device_status_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[3];
    uint8_t random_ptr[4];
    uint8_t random_len[4];
} cmd_rng_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[1];
    uint8_t key_id[2];
    uint8_t key_ptr[4];
    uint8_t aut_info_ptr[4];
} cmd_generate_key_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[3];
    uint8_t aut_info_ptr[4];
} cmd_failure_analysis_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[3];
    uint8_t aut_info_ptr[4];
} cmd_self_destroy_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[1];
    uint8_t key_id[2];
    uint8_t addr_ptr[4];
    uint8_t aut_info_ptr[4];
} cmd_seip_jump_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[15];
    uint8_t pin_code[16];
} cmd_seip_disable_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[3];
    uint8_t reg_addr[4];
    uint8_t reg_value[4];
    uint8_t reg_max_value[4];
    uint8_t reg_min_value[4];
} cmd_sensor_clk_cfg_t;

uint8_t cmd_get_device_info(uint16_t key_id, uint32_t *device_info_buf, uint8_t *mac_buf);

uint8_t cmd_get_device_status(uint16_t key_id, uint32_t *device_status_buf, uint8_t *mac_buf);

//key_id must != 0
uint8_t cmd_get_ske_hmac_256bit_key(uint16_t key_id, uint32_t *key, uint8_t *mac_buf);

uint8_t cmd_key_self_destroy(uint8_t *mac_buf);

uint8_t cmd_jump(uint32_t addr, uint8_t *mac_buf);

uint8_t cmd_disable_seip(void);

uint8_t cmd_sensor_clk_cfg(uint32_t reg_addr, uint32_t reg_value, uint32_t reg_max_value, uint32_t reg_min_value);

#endif