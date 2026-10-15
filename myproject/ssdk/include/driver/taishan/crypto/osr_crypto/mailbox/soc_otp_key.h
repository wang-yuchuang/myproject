/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef SOC_OTP_KEY_H
#define SOC_OTP_KEY_H

#include "cmd_common.h"

uint8_t *get_key_from_key_id(uint16_t key_id);

uint32_t KEK_encrypt(uint8_t *key_plain, uint32_t key_plain_bytes, uint16_t kek_keyid, uint8_t *key_cipher, uint8_t *mac_buf);

uint32_t KEK_decrypt(uint8_t *key_cipher, uint32_t key_plain_bytes, uint16_t kek_keyid, uint8_t *key_plain, uint8_t *mac_buf);

uint32_t otp_key_buf_test();

uint8_t get_otp_key_attribute(uint16_t key_id);

uint32_t check_key_id(uint16_t key_id, uint16_t is_aut_key);

uint16_t get_otp_key_mac_aut_key_id(uint16_t key_id);

uint32_t cmd_get_cmac(uint8_t mode, uint32_t key_type, uint16_t key_id, uint8_t *std_msg, uint32_t msg_bytes,
                      uint8_t *output_mac, uint8_t mac_bytes);

uint8_t get_otp_key_aut_mac(uint16_t *key_id, uint32_t key_count, uint8_t cmd[32], uint8_t *authentication_info);

#endif


























