/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <driver/taishan/crypto/osr_crypto/mailbox/cmd_basic_mgmt.h>
#define CMD_BASIC_GET_DEVICE_INFO_LEN 32

/* Function: get device info
 * Parameters:
 *     key_id --------------- input, internel key id, use for encrypt device info
 *     device_info_buff ---------------- output,
 * Return:
 *     BASIC_MGMT_SUCCESS(success); other(error)
 * Caution:
 */
uint8_t cmd_get_device_info(uint16_t key_id, uint32_t *device_info_buf, uint8_t *mac_buf)
{
    uint32_t cmd_buf[8];
    cmd_get_device_info_t *cmd_get_device_info = NULL;
    uint32_t ret;

    cmd_get_device_info = (cmd_get_device_info_t *)cmd_buf;
    cmd_get_device_info->cmd_id = GET_DEVICE_INFO;
    cmd_get_device_info->info_len = CMD_BASIC_GET_DEVICE_INFO_LEN;

    set_big_endian_2byte((uint8_t *)cmd_get_device_info->key_id, key_id);
    set_big_endian_4byte((uint8_t *)cmd_get_device_info->device_info_ptr, (uint32_t)device_info_buf);
    set_big_endian_4byte((uint8_t *)cmd_get_device_info->aut_info_ptr, (uint32_t)mac_buf);

    get_otp_key_aut_mac(&key_id, 1, (uint8_t *)cmd_buf, mac_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        print_buf_U8((uint8_t *)device_info_buf, 32, "device_info_buf");
    }
    else {
        printf("\r\n get device info failure, ret=%x\r\n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

/* Function: get device status
 * Parameters:
 *     key_id --------------- input, internel key id, use for encrypt device status
 *     device_status_buf ---------------- output,
 * Return:
 *     BASIC_MGMT_SUCCESS(success); other(error)
 * Caution:
 */
uint8_t cmd_get_device_status(uint16_t key_id, uint32_t *device_status_buf, uint8_t *mac_buf)
{
    uint32_t cmd_buf[8];
    cmd_get_device_status_t *cmd_get_device_status = NULL;
    uint32_t ret;

    cmd_get_device_status = (cmd_get_device_status_t *)cmd_buf;
    cmd_get_device_status->cmd_id = GET_DEVICE_STATUS;
    cmd_get_device_status->status_len = 16;

    set_big_endian_2byte((uint8_t *)cmd_get_device_status->key_id, key_id);
    set_big_endian_4byte((uint8_t *)cmd_get_device_status->device_status_ptr, (uint32_t)device_status_buf);
    set_big_endian_4byte((uint8_t *)cmd_get_device_status->aut_info_ptr, (uint32_t)mac_buf);

    get_otp_key_aut_mac(&key_id, 1, (uint8_t *)cmd_buf, mac_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        print_buf_U8((uint8_t *)device_status_buf, 16, "device_status_buf");
    }
    else {
        printf("\r\n get device status failure, ret=%x\r\n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

/* Function: get 256bit random key use for ske or hmac
 * Parameters:
 *     key_id --------------- input, internel key id, use for encrypt random key
 *     key ---------------- output,
 * Return:
 *     BASIC_MGMT_SUCCESS(success); other(error)
 * Caution:
 *     key_id must != 0
 */
uint8_t cmd_get_ske_hmac_256bit_key(uint16_t key_id, uint32_t *key, uint8_t *mac_buf)
{
    uint32_t cmd_buf[8];
    uint32_t ret;
    cmd_generate_key_t *cmd_get_key = NULL;

    cmd_get_key = (cmd_generate_key_t *)cmd_buf;
    cmd_get_key->cmd_id = GENERATE_KEY;

    set_big_endian_2byte((uint8_t *)cmd_get_key->key_id, key_id);
    set_big_endian_4byte((uint8_t *)cmd_get_key->key_ptr, (uint32_t)key);
    set_big_endian_4byte((uint8_t *)cmd_get_key->aut_info_ptr, (uint32_t)mac_buf);

    get_otp_key_aut_mac(&key_id, 1, (uint8_t *)cmd_buf, mac_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("\r\n get random key success \r\n");
        print_buf_U8((uint8_t *)key, 32, "key");
    }
    else {
        printf("\r\n get random key failure, ret=%x\r\n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

/* Function: destroy RSRK
 * Parameters:
 *
 * Return:
 *     BASIC_MGMT_SUCCESS(success); other(error)
 * Caution:
 *     this cmd only work when seip on high secure status
 */
uint8_t cmd_key_self_destroy(uint8_t *mac_buf)
{
    uint32_t cmd_buf[8];
    uint32_t ret;
    cmd_self_destroy_t *cmd_destroy = NULL;

    cmd_destroy = (cmd_self_destroy_t *)cmd_buf;
    cmd_destroy->cmd_id = SELF_DESTROY;

    set_big_endian_4byte((uint8_t *)cmd_destroy->aut_info_ptr, (uint32_t)mac_buf);

    ret  = cmd_get_cmac(CMAC_SKE_ALG_SM4, CMD_KEY_INTERNAL, 1, (uint8_t *)cmd_buf, 32, mac_buf, 16);

    if (CMD_RETURN_SUCESS != ret) {
        return SEIP_ERROR;
    }
    else
    {;}

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("\r\n SEIP destroy success\r\n");
    }
    else {
        printf("\r\n SEIP destroy failure, ret=%x\r\n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

/* Function: seip jump to addr run
 * Parameters:
 *     addr --------------- input, addr for seip run start
 * Return:
 *     BASIC_MGMT_SUCCESS(success); other(error)
 * Caution:
 *
 */
uint8_t cmd_jump(uint32_t addr, uint8_t *mac_buf)
{
    uint16_t key_id = 1;
    uint32_t cmd_buf[8];
    uint32_t ret;
    cmd_seip_jump_t *cmd_jump = NULL;

    cmd_jump = (cmd_seip_jump_t *)cmd_buf;
    cmd_jump->cmd_id = SEIP_JUMP;

    set_big_endian_2byte((uint8_t *)cmd_jump->key_id, key_id);
    set_big_endian_4byte((uint8_t *)cmd_jump->addr_ptr, (uint32_t)addr);
    set_big_endian_4byte((uint8_t *)cmd_jump->aut_info_ptr, (uint32_t)mac_buf);

    ret  = cmd_get_cmac(CMAC_SKE_ALG_SM4, CMD_KEY_INTERNAL, 1, (uint8_t *)cmd_buf, 32, mac_buf, 16);

    if (CMD_RETURN_SUCESS != ret) {
        return SEIP_ERROR;
    }
    else
    {;}

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("\r\n SEIP jump success\r\n");
    }
    else {
        printf("\r\n SEIP jump failure, ret=%x\r\n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

/* Function: disable seip
 * Parameters:
 *
 * Return:
 *     BASIC_MGMT_SUCCESS(success); other(error)
 * Caution:
 *
 */
uint8_t cmd_disable_seip(void)
{
    uint32_t cmd_buf[8];
    uint32_t ret;
    cmd_seip_disable_t *cmd_disable = NULL;
    const uint8_t pin_code[] = {
        0x32, 0x74, 0xDD, 0x6C, 0xD3, 0x26, 0xD7, 0x23, 0x65, 0xD8, 0xF8, 0x67, 0x92, 0xE7, 0x16, 0xE4,
    };

    cmd_disable = (cmd_seip_disable_t *)cmd_buf;
    cmd_disable->cmd_id = SEIP_DISABLE;

    memcpy_(cmd_disable->pin_code, (void *)pin_code, 16);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("\r\n SEIP disable success\r\n");
    }
    else {
        printf("\r\n SEIP disable failure, ret=%x\r\n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

/* Function: set sensor clk reg
 * Parameters:
 *     reg_addr --------------- input, sensor clk reg addr
 *     reg_value ---------------- input, value
 *     reg_max_value ---------------- input, max value
 *     reg_min_value ---------------- input, min value
 * Return:
 *     BASIC_MGMT_SUCCESS(success); other(error)
 * Caution:
 *
 */
uint8_t cmd_sensor_clk_cfg(uint32_t reg_addr, uint32_t reg_value, uint32_t reg_max_value, uint32_t reg_min_value)
{
    uint32_t cmd_buf[8];
    uint32_t ret;
    cmd_sensor_clk_cfg_t *cmd_sensor_cfg = NULL;

    cmd_sensor_cfg = (cmd_sensor_clk_cfg_t *)cmd_buf;
    cmd_sensor_cfg->cmd_id = SENSOR_CLK_CFG;

    set_big_endian_4byte((uint8_t *)cmd_sensor_cfg->reg_addr, (uint32_t)reg_addr);
    set_big_endian_4byte((uint8_t *)cmd_sensor_cfg->reg_value, (uint32_t)reg_value);
    set_big_endian_4byte((uint8_t *)cmd_sensor_cfg->reg_max_value, (uint32_t)reg_max_value);
    set_big_endian_4byte((uint8_t *)cmd_sensor_cfg->reg_min_value, (uint32_t)reg_min_value);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("\r\n sensor_clk_cfg success\r\n");
    }
    else {
        printf("\r\n sensor_clk_cfg failure, ret=%x\r\n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}
