/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <driver/taishan/crypto/osr_crypto/mailbox/cmd_hmac.h>

uint32_t cmd_hmac_generate_mac(uint8_t mode, uint8_t *msg, uint32_t msg_bytes, cmd_key_type_e key_type, const uint8_t *key, uint16_t key_id,
                               uint32_t key_bytes, uint8_t mac_bytes, uint8_t *out, uint8_t *mac_buf)
{

    uint16_t key_id_array[2];
    uint32_t cmd_buf[8];
    uint32_t ret;
    cmd_generateMAC_t *cmd_generate_hmac = NULL;

    /*********** generate hmac ***********/
    cmd_generate_hmac = (cmd_generateMAC_t *)cmd_buf;
    cmd_generate_hmac->cmd_id = GENERATE_MAC;
    cmd_generate_hmac->mode = mode;
    cmd_generate_hmac->mac_len = mac_bytes;

    set_big_endian_4byte((uint8_t *)cmd_generate_hmac->data_ptr, (uint32_t)msg);
    set_big_endian_4byte((uint8_t *)cmd_generate_hmac->data_len, (uint32_t)msg_bytes);
    set_big_endian_4byte((uint8_t *)cmd_generate_hmac->aut_info_ptr, (uint32_t)mac_buf);

    if (CMD_KEY_EXTERNAL_PLAINTEXT == key_type) {
        key_id_array[0] = 0;
        key_id_array[1] = 0;
        set_big_endian_4byte((uint8_t *)cmd_generate_hmac->key_ptr, (uint32_t)key);
    }
    else if (CMD_KEY_EXTERNAL_CIPHERTEXT == key_type) {
        key_id_array[0] = 0;
        key_id_array[1] = key_id;
        set_big_endian_4byte((uint8_t *)cmd_generate_hmac->key_ptr, (uint32_t)key);
    }
    else if (CMD_KEY_INTERNAL == key_type) {
        key_id_array[0] = key_id;
        key_id_array[1] = 0;
        set_big_endian_4byte((uint8_t *)cmd_generate_hmac->key_ptr, (uint32_t)0);
    }
    else {
        return CMD_RETURN_FAIL;
    }

    set_big_endian_2byte((uint8_t *)cmd_generate_hmac->key_id0, key_id_array[0]);
    set_big_endian_2byte((uint8_t *)cmd_generate_hmac->key_id1, key_id_array[1]);

    set_big_endian_2byte((uint8_t *)cmd_generate_hmac->key_len, (uint16_t)key_bytes);
    set_big_endian_4byte((uint8_t *)cmd_generate_hmac->mac_ptr, (uint32_t)out);

    get_otp_key_aut_mac(key_id_array, 2, (uint8_t *)cmd_buf, mac_buf);



    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if ((SEIP_SUCCESS == ret)) {

    }
    else {
        printf("generate hmac failure, mode=%d, ret=0x%x \n", mode, ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_hmac_verify_mac(uint8_t mode, uint8_t *msg, uint32_t msg_bytes, cmd_key_type_e key_type, const uint8_t *key, uint16_t key_id,
                             uint32_t key_bytes, uint8_t hmac_bytes, uint8_t *hmac, uint8_t *mac_buf)
{
    uint16_t key_id_array[2];
    uint32_t cmd_buf[8];
    uint32_t ret;
    cmd_verifyMAC_t *cmd_verify_hmac = NULL;

    /*********** verify mac ***********/
    cmd_verify_hmac = (cmd_verifyMAC_t *)cmd_buf;
    cmd_verify_hmac->cmd_id = VERIFY_MAC;
    cmd_verify_hmac->mode = mode;
    cmd_verify_hmac->mac_len = hmac_bytes;

    set_big_endian_4byte((uint8_t *)cmd_verify_hmac->data_ptr, (uint32_t)msg);
    set_big_endian_4byte((uint8_t *)cmd_verify_hmac->data_len, (uint32_t)msg_bytes);
    set_big_endian_4byte((uint8_t *)cmd_verify_hmac->aut_info_ptr, (uint32_t)mac_buf);

    if (CMD_KEY_EXTERNAL_PLAINTEXT == key_type) {
        key_id_array[0] = 0;
        key_id_array[1] = 0;
        set_big_endian_4byte((uint8_t *)cmd_verify_hmac->key_ptr, (uint32_t)key);
    }
    else if (CMD_KEY_EXTERNAL_CIPHERTEXT == key_type) {
        key_id_array[0] = 0;
        key_id_array[1] = key_id;
        set_big_endian_4byte((uint8_t *)cmd_verify_hmac->key_ptr, (uint32_t)key);
    }
    else if (CMD_KEY_INTERNAL == key_type) {
        key_id_array[0] = key_id;
        key_id_array[1] = 0;
        set_big_endian_4byte((uint8_t *)cmd_verify_hmac->key_ptr, (uint32_t)0);
    }
    else {
        return CMD_RETURN_FAIL;
    }

    set_big_endian_2byte((uint8_t *)cmd_verify_hmac->key_id0, key_id_array[0]);
    set_big_endian_2byte((uint8_t *)cmd_verify_hmac->key_id1, key_id_array[1]);

    set_big_endian_2byte((uint8_t *)cmd_verify_hmac->key_len, (uint16_t)key_bytes);
    set_big_endian_4byte((uint8_t *)cmd_verify_hmac->mac_ptr, (uint32_t)hmac);

    get_otp_key_aut_mac(key_id_array, 2, (uint8_t *)cmd_buf, mac_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if ((SEIP_SUCCESS == ret)) {

    }
    else {
        printf("generate hmac failure, mode=%d, ret=0x%x \n", mode, ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}