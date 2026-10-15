/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <driver/taishan/crypto/osr_crypto/mailbox/cmd_ske_basic.h>

uint32_t cmd_ske_basic_enc(uint8_t mode, uint8_t *plain, uint32_t bytelen, cmd_key_type_e key_type, uint8_t *key_buf,
                           uint16_t key_id, uint8_t *iv, uint8_t *cipher, uint8_t *mac_buf)
{
    uint16_t key_id_array[2];
    uint32_t cmd_buf[8] = {0};
    uint32_t ret;
    cmd_ske_encrypt_t *cmd_encrypt = NULL;

    /*********** encrypt ***********/
    cmd_encrypt = (cmd_ske_encrypt_t *)cmd_buf;
    cmd_encrypt->cmd_id = SKE_ENCRYPT;
    cmd_encrypt->need_seed = 1;
    cmd_encrypt->mode = mode;

    set_big_endian_4byte((uint8_t *)cmd_encrypt->iv_ptr, (uint32_t)iv);
    set_big_endian_4byte((uint8_t *)cmd_encrypt->data_ptr, (uint32_t)plain);
    set_big_endian_4byte((uint8_t *)cmd_encrypt->data_len, (uint32_t)bytelen);
    set_big_endian_4byte((uint8_t *)cmd_encrypt->aut_info_ptr, (uint32_t)mac_buf);

    if (CMD_KEY_EXTERNAL_PLAINTEXT == key_type) {
        key_id_array[0] = 0;
        key_id_array[1] = 0;
        set_big_endian_4byte((uint8_t *)cmd_encrypt->key_ptr, (uint32_t)key_buf);
    }
    else if (CMD_KEY_EXTERNAL_CIPHERTEXT == key_type) {
        key_id_array[0] = 0;
        key_id_array[1] = key_id;
        set_big_endian_4byte((uint8_t *)cmd_encrypt->key_ptr, (uint32_t)key_buf);
    }
    else if (CMD_KEY_INTERNAL == key_type) {
        key_id_array[0] = key_id;
        key_id_array[1] = 0;
        set_big_endian_4byte((uint8_t *)cmd_encrypt->key_ptr, (uint32_t)0);
    }
    else {
        return CMD_RETURN_FAIL;
    }

    set_big_endian_2byte((uint8_t *)cmd_encrypt->key_id0, key_id_array[0]);
    set_big_endian_2byte((uint8_t *)cmd_encrypt->key_id1, key_id_array[1]);
    set_big_endian_4byte((uint8_t *)cmd_encrypt->cipher_ptr, (uint32_t)cipher);

    get_otp_key_aut_mac(key_id_array, 2, (uint8_t *)cmd_buf, mac_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        /* add info print */
    }
    else {
        print_buf_U8((uint8_t *)cipher, bytelen, "cipher");
        printf("mailbox SKE encrypt failure, mode=%d, byteLen=%d,key_type=%d ret=0x%x \n", mode, bytelen, key_type, ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_ske_basic_dec(uint8_t mode, uint8_t *replain, uint32_t bytelen, cmd_key_type_e key_type, uint8_t *key_buf,
                           uint16_t key_id, uint8_t *iv, uint8_t *cipher, uint8_t *mac_buf)
{
    uint16_t key_id_array[2];
    uint32_t cmd_buf[8] = {0};
    uint32_t ret;
    cmd_ske_decrypt_t *cmd_decrypt = NULL;

    /*********** decrypt ***********/
    cmd_decrypt = (cmd_ske_decrypt_t *)cmd_buf;
    cmd_decrypt->cmd_id = SKE_DECRYPT;
    cmd_decrypt->need_seed = 1;
    cmd_decrypt->mode = mode;

    if (CMD_KEY_EXTERNAL_PLAINTEXT == key_type) {
        key_id_array[0] = 0;
        key_id_array[1] = 0;
        set_big_endian_4byte((uint8_t *)cmd_decrypt->key_ptr, (uint32_t)key_buf);
    }
    else if (CMD_KEY_EXTERNAL_CIPHERTEXT == key_type) {
        key_id_array[0] = 0;
        key_id_array[1] = key_id;
        set_big_endian_4byte((uint8_t *)cmd_decrypt->key_ptr, (uint32_t)key_buf);
    }
    else if (CMD_KEY_INTERNAL == key_type) {
        key_id_array[0] = key_id;
        key_id_array[1] = 0;
        set_big_endian_4byte((uint8_t *)cmd_decrypt->key_ptr, (uint32_t)0);
    }
    else {
        return CMD_RETURN_FAIL;
    }

    set_big_endian_2byte((uint8_t *)cmd_decrypt->key_id0, key_id_array[0]);
    set_big_endian_2byte((uint8_t *)cmd_decrypt->key_id1, key_id_array[1]);

    set_big_endian_4byte((uint8_t *)cmd_decrypt->iv_ptr, (uint32_t)iv);
    set_big_endian_4byte((uint8_t *)cmd_decrypt->cipher_ptr, (uint32_t)cipher);
    set_big_endian_4byte((uint8_t *)cmd_decrypt->cipher_len, (uint32_t)bytelen);
    set_big_endian_4byte((uint8_t *)cmd_decrypt->aut_info_ptr, (uint32_t)mac_buf);

    set_big_endian_4byte((uint8_t *)cmd_decrypt->data_ptr, (uint32_t)replain);

    get_otp_key_aut_mac(key_id_array, 2, (uint8_t *)cmd_buf, mac_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        /* add info print */
    }
    else {
        print_buf_U8((uint8_t *)cipher, bytelen, "cipher");
        print_buf_U8((uint8_t *)replain, bytelen, "replain");
        printf("mailbox SKE decrypt failure, mode=%d, byteLen=%d,key_type=%d ret=0x%x \n", mode, bytelen, key_type, ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}