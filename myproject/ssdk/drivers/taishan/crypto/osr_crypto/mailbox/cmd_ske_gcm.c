/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <driver/taishan/crypto/osr_crypto/mailbox/cmd_ske_gcm.h>

uint32_t cmd_ske_gcm_get_msg(uint8_t *plain, uint32_t plainlen, uint8_t *aad, uint8_t aadbytelen, uint8_t *msg, uint32_t *msgbytelen)
{
    uint8_t tmp_buf[16];

    *msgbytelen = 0;
    /*(aad + plaintext)*/
    uint32_copy((uint32_t *)msg, (uint32_t *)aad, (aadbytelen & (~0x0F) / 4));

    if (aadbytelen & 0x0F) {
        memcpy_(tmp_buf, aad + (aadbytelen & (~0x0F)), aadbytelen & 0x0F);
        memset_(tmp_buf + (aadbytelen & 0x0F), 0, 16 - (aadbytelen & 0x0F));
        uint32_copy((uint32_t *)msg + (aadbytelen & (~0x0F)) / 4, (uint32_t *)tmp_buf, 4);
        *msgbytelen += 4;
    }

    uint32_copy((uint32_t *)msg + (((*msgbytelen + 15) / 16) * 4), (uint32_t *)plain, (plainlen & (~0x0F)) / 4);

    if (plainlen & 0x0F) {
        memcpy_(tmp_buf, plain + (plainlen & (~0x0F)), plainlen & 0x0F);
        memset_(tmp_buf + (plainlen & 0x0F), 0, 16 - (plainlen & 0x0F));
        uint32_copy((uint32_t *)msg + (((aadbytelen + 15) / 16) * 4) + (plainlen & (~0x0F)) / 4, (uint32_t *)tmp_buf, 4);
        *msgbytelen += 4;
    }

    *msgbytelen += (aadbytelen + plainlen);

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_ske_gcm_enc(uint8_t mode, uint8_t *msg, uint32_t msgbytelen, cmd_key_type_e key_type, uint8_t *key,
                         uint16_t key_id, uint8_t *iv, uint8_t ivbytelen, uint8_t aadbytelen,
                         uint8_t *cipher, uint32_t tagbytelen, uint8_t *mac_buf, gcm_enc_input_ptr_t *cmd_gcm_enc_input)
{
    uint16_t key_id_array[2];
    uint32_t cmd_buf[8];
    uint32_t ret;
    cmd_aead_encrypt_gcm_t *cmd_gcm_encrypt = NULL;

    /*********** encrypt ***********/
    cmd_gcm_encrypt = (cmd_aead_encrypt_gcm_t *)cmd_buf;
    cmd_gcm_encrypt->cmd_id = SKE_AEAD_ENCRYPT_GCM;
    cmd_gcm_encrypt->mode = mode;

    set_big_endian_4byte((uint8_t *)cmd_gcm_enc_input->aad_data_ptr, (uint32_t)msg);
    set_big_endian_4byte((uint8_t *)cmd_gcm_enc_input->iv_ptr, (uint32_t)iv);
    set_big_endian_4byte((uint8_t *)cmd_gcm_encrypt->gcm_enc_input_ptr, (uint32_t)cmd_gcm_enc_input);
    set_big_endian_2byte((uint8_t *)cmd_gcm_encrypt->iv_len, (uint32_t)ivbytelen);
    set_big_endian_2byte((uint8_t *)cmd_gcm_encrypt->aad_len, (uint32_t)aadbytelen);
    set_big_endian_4byte((uint8_t *)cmd_gcm_encrypt->data_len, (uint32_t)msgbytelen);
    cmd_gcm_encrypt->tag_len = tagbytelen;
    set_big_endian_4byte((uint8_t *)cmd_gcm_encrypt->aut_info_ptr, (uint32_t)mac_buf);

    if (CMD_KEY_EXTERNAL_PLAINTEXT == key_type) {
        key_id_array[0] = 0;
        key_id_array[1] = 0;
        set_big_endian_4byte((uint8_t *)cmd_gcm_encrypt->key_ptr, (uint32_t)key);
    }
    else if (CMD_KEY_EXTERNAL_CIPHERTEXT == key_type) {
        key_id_array[0] = 0;
        key_id_array[1] = key_id;
        set_big_endian_4byte((uint8_t *)cmd_gcm_encrypt->key_ptr, (uint32_t)key);
    }
    else if (CMD_KEY_INTERNAL == key_type) {
        key_id_array[0] = key_id;
        key_id_array[1] = 0;
        set_big_endian_4byte((uint8_t *)cmd_gcm_encrypt->key_ptr, (uint32_t)0);
    }
    else {
        return CMD_RETURN_FAIL;
    }

    set_big_endian_2byte((uint8_t *)cmd_gcm_encrypt->key_id0, key_id_array[0]);
    set_big_endian_2byte((uint8_t *)cmd_gcm_encrypt->key_id1, key_id_array[1]);
    set_big_endian_4byte((uint8_t *)cmd_gcm_encrypt->cipher_tag_ptr, (uint32_t)cipher);

    get_otp_key_aut_mac(key_id_array, 2, (uint8_t *)cmd_buf, mac_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {

    }
    else {
        print_buf_U8((uint8_t *)cipher, msgbytelen + 16, "cipher + tag");
        printf("SKE gcm encrypt failure, mode=%d, ret=0x%x \n", mode, ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_ske_gcm_dec(uint8_t mode, uint8_t *cipher, uint32_t cipherbytelen, cmd_key_type_e key_type, uint8_t *key,
                         uint16_t key_id, uint8_t *iv, uint8_t ivbytelen, uint8_t aadbytelen,
                         uint8_t *msg, uint8_t *tag, uint32_t tagbytelen, uint8_t *mac_buf, gcm_dec_input_ptr_t *cmd_gcm_dec_input)
{
    uint8_t tag_buf[16];
    uint16_t key_id_array[2];
    uint32_t ret;
    uint32_t cmd_buf[8];
    cmd_aead_decrypt_gcm_t *cmd_gcm_decrypt = NULL;

    /*********** decrypt ***********/
    cmd_gcm_decrypt = (cmd_aead_decrypt_gcm_t *)cmd_buf;
    cmd_gcm_decrypt->cmd_id = SKE_AEAD_DECRYPT_GCM;
    cmd_gcm_decrypt->mode = mode;

    if (CMD_KEY_EXTERNAL_PLAINTEXT == key_type) {
        key_id_array[0] = 0;
        key_id_array[1] = 0;
        set_big_endian_4byte((uint8_t *)cmd_gcm_decrypt->key_ptr, (uint32_t)key);
    }
    else if (CMD_KEY_EXTERNAL_CIPHERTEXT == key_type) {
        key_id_array[0] = 0;
        key_id_array[1] = key_id;
        set_big_endian_4byte((uint8_t *)cmd_gcm_decrypt->key_ptr, (uint32_t)key);
    }
    else if (CMD_KEY_INTERNAL == key_type) {
        key_id_array[0] = key_id;
        key_id_array[1] = 0;
        set_big_endian_4byte((uint8_t *)cmd_gcm_decrypt->key_ptr, (uint32_t)0);
    }
    else {
        return CMD_RETURN_FAIL;
    }

    set_big_endian_2byte((uint8_t *)cmd_gcm_decrypt->key_id0, key_id_array[0]);
    set_big_endian_2byte((uint8_t *)cmd_gcm_decrypt->key_id1, key_id_array[1]);

    set_big_endian_4byte((uint8_t *)cmd_gcm_dec_input->aad_cipher_ptr, (uint32_t)cipher);
    set_big_endian_4byte((uint8_t *)cmd_gcm_dec_input->iv_ptr, (uint32_t)iv);
    set_big_endian_4byte((uint8_t *)cmd_gcm_dec_input->tag_ptr, (uint32_t)tag);
    set_big_endian_4byte((uint8_t *)cmd_gcm_decrypt->gcm_dec_input_ptr, (uint32_t)cmd_gcm_dec_input);
    set_big_endian_2byte((uint8_t *)cmd_gcm_decrypt->iv_len, (uint32_t)ivbytelen);
    set_big_endian_2byte((uint8_t *)cmd_gcm_decrypt->aad_len, (uint32_t)aadbytelen);
    set_big_endian_4byte((uint8_t *)cmd_gcm_decrypt->cipher_len, (uint32_t)cipherbytelen);
    cmd_gcm_decrypt->tag_len = tagbytelen;
    set_big_endian_4byte((uint8_t *)cmd_gcm_decrypt->aut_info_ptr, (uint32_t)mac_buf);

    set_big_endian_4byte((uint8_t *)cmd_gcm_decrypt->data_tag_ptr, (uint32_t)msg);

    get_otp_key_aut_mac(key_id_array, 2, (uint8_t *)cmd_buf, mac_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
    }
    else {
        print_buf_U8((uint8_t *)msg, (cipherbytelen - aadbytelen + 16), "plain + tag");
        printf("SKE gcm decrypt failure, mode=%d, ret=0x%x \n", mode, ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}
