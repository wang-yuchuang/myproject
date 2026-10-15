/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <driver/taishan/crypto/osr_crypto/mailbox/soc_otp_key.h>
#include <driver/taishan/crypto/osr_crypto/mailbox/cmd_hmac.h>
#include <driver/taishan/crypto/osr_crypto/mailbox/cmd_ske_basic.h>

/**************************** the OTP key *****************************
  key buffer:(first word to last word)
  caution: for the following every key, it is the same as SKE key register
107119e7 5fe88b1e 8922e4ae ceae16cf 00000000 00000000 00000000 00000000
0ee9f560 9a22ecba a3a43744 8a9d9b8d 00000000 00000000 00000000 00000000
f30e2676 eac1222c 02e596c5 79a8eb9f 00000000 00000000 00000000 00000000
45a4079f 92fcde09 2a553d1f 8e06c0cb 00000000 00000000 00000000 00000000
104bab38 ebe73c79 1cb5491c 52e8dc31 00000000 00000000 00000000 00000000
8d7972b0 cccb6db2 36948442 c4f5b5f0 697770aa 08a35e00 bc9c08b3 362d70f8
95d63843 c1698e12 5d6dec1b 1cf9cfe3 00000000 00000000 00000000 00000000
3cacc647 ec7f7074 fa2d7f2f 55b12764 51c6c3f2 7e542a12 922d9146 55711916

  for AES128/SM4(standard byte stream): first half ---- second half
key1: CF16AECEAEE422891E8BE85FE7197110 00000000000000000000000000000000
key2: 8D9B9D8A4437A4A3BAEC229A60F5E90E 00000000000000000000000000000000
key3: 9FEBA879C596E5022C22C1EA76260EF3 00000000000000000000000000000000
key4: CBC0068E1F3D552A09DEFC929F07A445 00000000000000000000000000000000
key5: 31DCE8521C49B51C793CE7EB38AB4B10 00000000000000000000000000000000
key6: F0B5F5C442849436B26DCBCCB072798D F8702D36B3089CBC005EA308AA707769
key7: E3CFF91C1BEC6D5D128E69C14338D695 00000000000000000000000000000000
key8: 6427B1552F7F2DFA74707FEC47C6AC3C 1619715546912D92122A547EF2C3C651

  for DES(standard byte stream): first half ---- second half
key1: 1E8BE85FE7197110 0000000000000000
key2: BAEC229A60F5E90E 0000000000000000
key3: 2C22C1EA76260EF3 0000000000000000
key4: 09DEFC929F07A445 0000000000000000
key5: 793CE7EB38AB4B10 0000000000000000
key6: B26DCBCCB072798D 005EA308AA707769
key7: 128E69C14338D695 0000000000000000
key8: 74707FEC47C6AC3C 122A547EF2C3C651
***********************************************************************/
#define USE_OTP_KEY_PLAIN 1
/*must be equal to seip config*/
/*OTP key attribute*/
#define OTP_KEY_ATR_FREE_USE        (0x00)
#define OTP_KEY_ATR_MAC_AUT         (0x02)
#define OTP_KEY_ATR_DISUSE          (0x03)
#define OTP_KEY_AUT_KEY             (0x20)
#define SSK_CTRL0                   0x20009100
#define SSK_CTRL1                   0x81910000

uint8_t OTP_KEY_BUFFER[] __CACHE_ALIGN = {
    0xCF, 0x16, 0xAE, 0xCE, 0xAE, 0xE4, 0x22, 0x89, 0x1E, 0x8B, 0xE8, 0x5F, 0xE7, 0x19, 0x71, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x8D, 0x9B, 0x9D, 0x8A, 0x44, 0x37, 0xA4, 0xA3, 0xBA, 0xEC, 0x22, 0x9A, 0x60, 0xF5, 0xE9, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x9F, 0xEB, 0xA8, 0x79, 0xC5, 0x96, 0xE5, 0x02, 0x2C, 0x22, 0xC1, 0xEA, 0x76, 0x26, 0x0E, 0xF3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xCB, 0xC0, 0x06, 0x8E, 0x1F, 0x3D, 0x55, 0x2A, 0x09, 0xDE, 0xFC, 0x92, 0x9F, 0x07, 0xA4, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x31, 0xDC, 0xE8, 0x52, 0x1C, 0x49, 0xB5, 0x1C, 0x79, 0x3C, 0xE7, 0xEB, 0x38, 0xAB, 0x4B, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xF0, 0xB5, 0xF5, 0xC4, 0x42, 0x84, 0x94, 0x36, 0xB2, 0x6D, 0xCB, 0xCC, 0xB0, 0x72, 0x79, 0x8D, 0xF8, 0x70, 0x2D, 0x36, 0xB3, 0x08, 0x9C, 0xBC, 0x00, 0x5E, 0xA3, 0x08, 0xAA, 0x70, 0x77, 0x69,
    0xE3, 0xCF, 0xF9, 0x1C, 0x1B, 0xEC, 0x6D, 0x5D, 0x12, 0x8E, 0x69, 0xC1, 0x43, 0x38, 0xD6, 0x95, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x64, 0x27, 0xB1, 0x55, 0x2F, 0x7F, 0x2D, 0xFA, 0x74, 0x70, 0x7F, 0xEC, 0x47, 0xC6, 0xAC, 0x3C, 0x16, 0x19, 0x71, 0x55, 0x46, 0x91, 0x2D, 0x92, 0x12, 0x2A, 0x54, 0x7E, 0xF2, 0xC3, 0xC6, 0x51,
};

uint8_t *get_key_from_key_id(uint16_t key_id)
{
    uint8_t *addr = NULL;

    addr = OTP_KEY_BUFFER + ((key_id - 1) & 0x7FFF) * 32;

    if (key_id & 0x8000) {
        addr += 16;
    }

    return addr;
}

uint32_t KEK_encrypt(uint8_t *key_plain, uint32_t key_plain_bytes, uint16_t kek_keyid, uint8_t *key_cipher, uint8_t *mac_buf)
{
    uint8_t *key;
    uint16_t key_id_array[2];
    uint32_t ret;
    uint32_t cmd_buf[8] = {0};

    cmd_ske_encrypt_t *cmd_encrypt = NULL;
    uint32_t *key_plain_temp;
    uint32_t *key_cipher_temp;

    key_plain_temp = (uint32_t *)key_plain;
    key_cipher_temp = (uint32_t *)key_cipher;

    if (key_plain_bytes & 0x0F) {
        key_plain_bytes += 16 - (key_plain_bytes & 0x0F);
    }
    else
    {;}

#ifdef USE_OTP_KEY_PLAIN
    key = OTP_KEY_BUFFER + ((kek_keyid & 0x7FFF) - 1) * 32;

    if (kek_keyid & 0x8000) {
        key += 16;
    }

#endif

    /*********** encrypt ***********/
    cmd_encrypt = (cmd_ske_encrypt_t *)cmd_buf;
    cmd_encrypt->cmd_id = SKE_ENCRYPT;
    cmd_encrypt->need_seed = 1;
    cmd_encrypt->mode = SKE_ALG_SM4_ECB;

    set_big_endian_4byte((uint8_t *)cmd_encrypt->iv_ptr, 0x0);
    set_big_endian_4byte((uint8_t *)cmd_encrypt->data_ptr, (uint32_t)key_plain_temp);
    set_big_endian_4byte((uint8_t *)cmd_encrypt->data_len, key_plain_bytes);
    set_big_endian_4byte((uint8_t *)cmd_encrypt->aut_info_ptr, (uint32_t)mac_buf);

    key_id_array[0] = kek_keyid;
    key_id_array[1] = 0;
    set_big_endian_4byte((uint8_t *)cmd_encrypt->key_ptr, (uint32_t)0);

    set_big_endian_2byte((uint8_t *)cmd_encrypt->key_id0, key_id_array[0]);
    set_big_endian_2byte((uint8_t *)cmd_encrypt->key_id1, key_id_array[1]);
    set_big_endian_4byte((uint8_t *)cmd_encrypt->cipher_ptr, (uint32_t)key_cipher_temp);

    get_otp_key_aut_mac(key_id_array, 2, (uint8_t *)cmd_buf, mac_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS != ret) {
        printf("KEK_encrypt fail \n");
        return SEIP_ERROR;
    }
    else {
        return CMD_RETURN_SUCESS;
    }
}

uint32_t KEK_decrypt(uint8_t *key_cipher, uint32_t key_plain_bytes, uint16_t kek_keyid, uint8_t *key_plain, uint8_t *mac_buf)
{
    uint8_t *key;

    uint32_t ret;
    uint16_t key_id_array[2];
    uint32_t cmd_buf[8] = {0};

    cmd_ske_decrypt_t *cmd_decrypt = NULL;

    if (key_plain_bytes & 0x0F) {
        key_plain_bytes += 16 - (key_plain_bytes & 0x0F);
    }
    else
    {;}

#ifdef USE_OTP_KEY_PLAIN
    key = OTP_KEY_BUFFER + ((kek_keyid & 0x7FFF) - 1) * 32;

    if (kek_keyid & 0x8000) {
        key += 16;
    }

#endif

    /*********** decrypt ***********/
    cmd_decrypt = (cmd_ske_decrypt_t *)cmd_buf;
    cmd_decrypt->cmd_id = SKE_DECRYPT;
    cmd_decrypt->need_seed = 1;
    cmd_decrypt->mode = SKE_ALG_SM4_ECB;

    set_big_endian_4byte((uint8_t *)cmd_decrypt->iv_ptr, 0x0);
    set_big_endian_4byte((uint8_t *)cmd_decrypt->cipher_ptr, (uint32_t)key_cipher);
    set_big_endian_4byte((uint8_t *)cmd_decrypt->cipher_len, (uint32_t)key_plain_bytes);
    set_big_endian_4byte((uint8_t *)cmd_decrypt->aut_info_ptr, (uint32_t)mac_buf);
    /*the key paras keep the same as the encrypt test.*/

    key_id_array[0] = kek_keyid;
    key_id_array[1] = 0;
    set_big_endian_4byte((uint8_t *)cmd_decrypt->key_ptr, (uint32_t)0);

    set_big_endian_2byte((uint8_t *)cmd_decrypt->key_id0, key_id_array[0]);
    set_big_endian_2byte((uint8_t *)cmd_decrypt->key_id1, key_id_array[1]);

    set_big_endian_4byte((uint8_t *)cmd_decrypt->data_ptr, (uint32_t)key_plain);

    get_otp_key_aut_mac(key_id_array, 2, (uint8_t *)cmd_buf, mac_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS != ret) {
        printf("KEK_decrypt fail \n");
        return SEIP_ERROR;
    }
    else {
        return CMD_RETURN_SUCESS;
    }
}

uint32_t otp_key_buf_test()
{
    uint32_t i;
    uint8_t buf[32 * 8];

    for (i = 0; i < 8; i++) {
        memcpy_(buf + i * 32, get_key_from_key_id(0x0000 + i + 1), 16);
        memcpy_(buf + i * 32 + 16, get_key_from_key_id(0x8000 + i + 1), 16);
    }

    if (memcmp_(buf, OTP_KEY_BUFFER, 32 * 8)) {
        printf("CACC key buffer reading failure \n");
        print_buf_U8(buf, 32 * 8, "otp key");
    }
    else {
        printf("CACC key buffer reading success \n");
    }

    return 0;
}

uint8_t get_otp_key_attribute(uint16_t key_id)
{
    uint32_t tmp;

    key_id = key_id & 0x7FFF;

    if (key_id < 5) {
        tmp = SSK_CTRL0;
    }
    else {
        tmp = SSK_CTRL1;
    }

    key_id = (key_id - 1) & 3;
    tmp = tmp >> ((3 - key_id) * 8);

    return (uint8_t)(tmp & 0xFF);
}


uint32_t check_key_id(uint16_t key_id, uint16_t is_aut_key)
{
    uint8_t attr, tmp;

    if ((KEYID_HALF_MASK == key_id) || ((key_id & 0x7FFF) > KEYID_MAX)) {
        return SEIP_ERROR;
    }
    else if (0 == key_id) {
        return SEIP_SUCCESS;
    }
    else
    {;}

    attr = get_otp_key_attribute(key_id);

    if (is_aut_key) {
        if (0 == (attr & OTP_KEY_AUT_KEY)) {
            return SEIP_ERROR;
        }
        else
        {;}
    }
    else {
        if (0 != (attr & OTP_KEY_AUT_KEY)) {
            return SEIP_ERROR;
        }
        else
        {;}

        tmp = attr >> 6;

        switch (tmp) {
            case OTP_KEY_ATR_FREE_USE:
            case OTP_KEY_ATR_MAC_AUT:
                break;

            default:
                return SEIP_ERROR;
        }
    }

    return SEIP_SUCCESS;
}

uint16_t get_otp_key_mac_aut_key_id(uint16_t key_id)
{
    uint16_t tmp, mac_key_id;

    tmp = get_otp_key_attribute(key_id);
    mac_key_id = tmp & 0x0F;

    if (0 == (mac_key_id & 0x0F)) {
        return 0;
    }

    if (tmp & 0x10) {
        mac_key_id |= 0x8000;
    }

    if (SEIP_ERROR == check_key_id(mac_key_id, 1)) {
        mac_key_id = 0;
    }

    return mac_key_id;
}

#define SOC_OTP_KEY_GET_MAC_BUFF_LEN 128
uint32_t get_mac_buff[SOC_OTP_KEY_GET_MAC_BUFF_LEN] __CACHE_ALIGN;

uint32_t cmd_get_cmac(uint8_t mode, uint32_t key_type, uint16_t key_id, uint8_t *std_msg, uint32_t msg_bytes,
                      uint8_t *output_mac, uint8_t mac_bytes)
{
    uint8_t tmp_buf[16];
    uint8_t *key;

    uint32_t *msg = (uint32_t *)get_mac_buff;
    uint32_t *tmp_out = msg + 0x20;

    uint16_t key_id_array[2];
    uint32_t cmd_buf[8];

    cmd_generateMAC_t *cmd_generate_ske_mac = NULL;

    uint32_t keyByteLen;
    uint32_t ret;

    /* prepare data */
    keyByteLen = cmd_get_ske_key_bytes(mode);

    if (0 == keyByteLen) {
        return 1;
    }

    /* set plain */
    uint32_clear(msg, 0x40);
    uint32_copy(msg, (uint32_t *)std_msg, (msg_bytes & (~0x0F)) / 4);

    if (msg_bytes & 0x0F) {
        memcpy_(tmp_buf, (void *)(std_msg + (msg_bytes & (~0x0F))), msg_bytes & 0x0F);
        memset_(tmp_buf + (msg_bytes & 0x0F), 0, 16 - (msg_bytes & 0x0F));
        uint32_copy(msg + (msg_bytes & (~0x0F)) / 4, (uint32_t *)tmp_buf, 4);
    }

    /*********** generate ske mac ***********/
    cmd_generate_ske_mac = (cmd_generateMAC_t *)cmd_buf;
    cmd_generate_ske_mac->cmd_id = GENERATE_MAC;
    cmd_generate_ske_mac->need_seed = 1;
    cmd_generate_ske_mac->mode = mode;
    cmd_generate_ske_mac->mac_len = mac_bytes;
    set_big_endian_4byte((uint8_t *)cmd_generate_ske_mac->aut_info_ptr, (uint32_t)0);

    set_big_endian_4byte((uint8_t *)cmd_generate_ske_mac->data_ptr, (uint32_t)msg);
    set_big_endian_4byte((uint8_t *)cmd_generate_ske_mac->data_len, (uint32_t)msg_bytes);

    key_id_array[0] = 0;
    key_id_array[1] = 0;

    key = OTP_KEY_BUFFER + ((key_id & 0x7FFF) - 1) * 32;

    if (key_id & 0x8000) {
        key += 16;
    }

    set_big_endian_4byte((uint8_t *)cmd_generate_ske_mac->key_ptr, (uint32_t)key);
    set_big_endian_2byte((uint8_t *)cmd_generate_ske_mac->key_id0, key_id_array[0]);
    set_big_endian_2byte((uint8_t *)cmd_generate_ske_mac->key_id1, key_id_array[1]);
    set_big_endian_4byte((uint8_t *)cmd_generate_ske_mac->mac_ptr, (uint32_t)tmp_out);

    arch_clean_cache_range((addr_t)msg, msg_bytes);
    arch_invalidate_cache_range((addr_t)tmp_out, mac_bytes);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if ((SEIP_SUCCESS == ret)) {
        if (output_mac) {
            memcpy_(output_mac, tmp_out, mac_bytes);
        }
        else
        {;}
    }
    else {
        print_buf_U8((uint8_t *)msg, msg_bytes, "msg");
        print_buf_U8((uint8_t *)tmp_out, (msg_bytes + 15) & (~0x0F), "output");
        printf("\r\n SKE generate mac failure, mode=%d, ret=%x\r\n", mode, ret);
        return 1;
    }

    return CMD_RETURN_SUCESS;
}

uint8_t get_otp_key_aut_mac(uint16_t *key_id, uint32_t key_count, uint8_t cmd[32], uint8_t *authentication_info)
{
    uint32_t i, ret;
    uint16_t mac_key_id;
    uint8_t tmp;

    for (i = 0; i < key_count; i++) {
        if (0 == key_id[i]) {
            continue;
        }
        else {
            tmp = get_otp_key_attribute(key_id[i]);
            tmp >>= 6;

            if (OTP_KEY_ATR_FREE_USE == tmp) {
                continue;
            }

            mac_key_id = get_otp_key_mac_aut_key_id(key_id[i]);

            if (0 == mac_key_id) {
                return SEIP_ERROR;
            }
            else
            {;}

            ret  = cmd_get_cmac(CMAC_SKE_ALG_SM4, CMD_KEY_EXTERNAL_PLAINTEXT, mac_key_id, cmd, 32, authentication_info + i * 16, 16);

            if (CMD_RETURN_SUCESS != ret) {
                return SEIP_ERROR;
            }
            else
            {;}
        }
    }

    return 0;
}