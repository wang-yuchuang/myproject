/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <driver/taishan/crypto/osr_crypto/mailbox/cmd_rsa.h>

uint32_t cmd_rsa_get_prime(uint16_t primebitlen, uint8_t *prime)
{
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_rsa_getprime_t *cmd_get_prime = NULL;

    /*********** generate random prime ***********/
    cmd_get_prime = (cmd_rsa_getprime_t *)cmd_buf;
    cmd_get_prime->cmd_id = RSA_GENERATE_PRIME;

    set_big_endian_2byte((uint8_t *)cmd_get_prime->prime_len, (uint32_t)primebitlen);
    set_big_endian_4byte((uint8_t *)cmd_get_prime->prime_ptr, (uint32_t)prime);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("RSA_GENERATE_PRIME success(%d bits) \n", primebitlen);

    }
    else {
        printf("RSA_GENERATE_PRIME error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_rsa_generate_key(uint16_t ebitlen, uint16_t nbitlen, cmd_key_type_e generate_key_type, uint16_t generate_key_id,
                              uint8_t *n, uint8_t *e, uint8_t *d, uint8_t *mac_buf)
{
    uint16_t key_id_array[2];
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_rsa_generatekey_t *cmd_rsa_get_key = NULL;

    /*********** generate key pair ***********/
    cmd_rsa_get_key = (cmd_rsa_generatekey_t *)cmd_buf;
    cmd_rsa_get_key->cmd_id = RSA_GENERATE_KEY;

    set_big_endian_2byte((uint8_t *)cmd_rsa_get_key->e_len, (uint32_t)ebitlen);
    set_big_endian_2byte((uint8_t *)cmd_rsa_get_key->n_len, (uint32_t)nbitlen);

    if (CMD_KEY_EXTERNAL_PLAINTEXT == generate_key_type) {
        key_id_array[0] = 0;
    }
    else if (CMD_KEY_EXTERNAL_CIPHERTEXT == generate_key_type) {
        key_id_array[0] = generate_key_id;
    }

    set_big_endian_2byte((uint8_t *)cmd_rsa_get_key->key_id, key_id_array[0]);

    set_big_endian_4byte((uint8_t *)cmd_rsa_get_key->e_ptr, (uint32_t)e);
    set_big_endian_4byte((uint8_t *)cmd_rsa_get_key->n_ptr, (uint32_t)n);
    set_big_endian_4byte((uint8_t *)cmd_rsa_get_key->d_ptr, (uint32_t)d);
    set_big_endian_4byte((uint8_t *)cmd_rsa_get_key->aut_info_ptr, (uint32_t)mac_buf);

    get_otp_key_aut_mac(key_id_array, 1, (uint8_t *)cmd_buf, mac_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("RSA_GENERATE_KEY eBitLen=%d, nBitLen=%d success \n", ebitlen, nbitlen);
    }
    else {
        printf("RSA_GENERATE_KEY eBitLen=%d, nBitLen=%d error, ret = 0x%x \n", ebitlen, nbitlen, ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

//msg_len is same with n bitlen
uint32_t cmd_rsa_encrypt(uint8_t *msg, uint8_t *e, uint8_t *n, uint32_t nbitlen, uint8_t *out)
{
    uint16_t key_id_array[2];
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_rsa_encrypt_t *cmd_rsa_enc = NULL;

    /*********** encrypt ***********/
    cmd_rsa_enc = (cmd_rsa_encrypt_t *)cmd_buf;
    cmd_rsa_enc->cmd_id = RSA_ENCRYPT;

    set_big_endian_4byte((uint8_t *)cmd_rsa_enc->data_ptr, (uint32_t)msg);
    set_big_endian_4byte((uint8_t *)cmd_rsa_enc->e_ptr, (uint32_t)e);
    set_big_endian_4byte((uint8_t *)cmd_rsa_enc->cipher_ptr, (uint32_t)out);
    set_big_endian_4byte((uint8_t *)cmd_rsa_enc->n_ptr, (uint32_t)n);
    set_big_endian_2byte((uint8_t *)cmd_rsa_enc->n_len, (uint32_t)nbitlen);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("enc %d pass \n", nbitlen);
    }
    else {
        printf("enc %d error, ret = 0x%x \n", nbitlen, ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_rsa_decrypt(uint8_t *msg, uint8_t *d, uint8_t *e, uint8_t *n, uint32_t nbitlen, uint8_t *out, cmd_key_type_e decrypt_key_type, uint16_t decrypt_key_id, uint8_t *mac_buf)
{
    uint16_t key_id_array[2];
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_rsa_decrypt_t *cmd_rsa_dec = NULL;

    /*********** decrypt ***********/
    cmd_rsa_dec = (cmd_rsa_decrypt_t *)cmd_buf;
    cmd_rsa_dec->cmd_id = RSA_DECRYPT;

    cmd_rsa_dec->need_high_sec_ver = 1;

    set_big_endian_4byte((uint8_t *)cmd_rsa_dec->data_ptr, (uint32_t)msg);
    set_big_endian_4byte((uint8_t *)cmd_rsa_dec->e_ptr, (uint32_t)e);
    set_big_endian_4byte((uint8_t *)cmd_rsa_dec->cipher_ptr, (uint32_t)out);

    set_big_endian_4byte((uint8_t *)cmd_rsa_dec->d_ptr, (uint32_t)d);

    if (CMD_KEY_EXTERNAL_PLAINTEXT == decrypt_key_type) {
        key_id_array[0] = 0;
    }
    else if (CMD_KEY_EXTERNAL_CIPHERTEXT == decrypt_key_type) {
        key_id_array[0] = decrypt_key_id;
    }
    else {
        return CMD_RETURN_FAIL;
    }

    set_big_endian_2byte((uint8_t *)cmd_rsa_dec->key_id, key_id_array[0]);
    set_big_endian_4byte((uint8_t *)cmd_rsa_dec->n_ptr, (uint32_t)n);
    set_big_endian_2byte((uint8_t *)cmd_rsa_dec->n_len, (uint32_t)nbitlen);
    set_big_endian_4byte((uint8_t *)cmd_rsa_dec->aut_info_ptr, (uint32_t)mac_buf);

    get_otp_key_aut_mac(key_id_array, 1, (uint8_t *)cmd_buf, mac_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("dec pass \n");
    }
    else {
        printf("dec error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_rsa_generate_signature(uint8_t *msg, uint8_t *d, uint8_t *e, uint8_t *n, uint32_t nbitlen, uint8_t *out, cmd_key_type_e sign_key_type, uint16_t sign_key_id, uint8_t *mac_buf)
{
    uint16_t key_id_array[2];
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_rsa_generate_signature_t *cmd_rsa_sign = NULL;

    /*********** sign ***********/
    cmd_rsa_sign = (cmd_rsa_generate_signature_t *)cmd_buf;
    cmd_rsa_sign->cmd_id = RSA_GENERATE_SIGNATURE;

    cmd_rsa_sign->need_high_sec_ver = 1;

    set_big_endian_4byte((uint8_t *)cmd_rsa_sign->data_ptr, (uint32_t)msg);
    set_big_endian_4byte((uint8_t *)cmd_rsa_sign->e_ptr, (uint32_t)e);
    set_big_endian_4byte((uint8_t *)cmd_rsa_sign->signature_ptr, (uint32_t)out);

    if (CMD_KEY_EXTERNAL_PLAINTEXT == sign_key_type) {
        key_id_array[0] = 0;
    }
    else if (CMD_KEY_EXTERNAL_CIPHERTEXT == sign_key_type) {
        key_id_array[0] = sign_key_id;
    }
    else {
        return CMD_RETURN_FAIL;
    }

    set_big_endian_4byte((uint8_t *)cmd_rsa_sign->d_ptr, (uint32_t)d);
    set_big_endian_2byte((uint8_t *)cmd_rsa_sign->key_id, key_id_array[0]);
    set_big_endian_4byte((uint8_t *)cmd_rsa_sign->n_ptr, (uint32_t)n);
    set_big_endian_2byte((uint8_t *)cmd_rsa_sign->n_len, (uint32_t)nbitlen);
    set_big_endian_4byte((uint8_t *)cmd_rsa_sign->aut_info_ptr, (uint32_t)mac_buf);

    get_otp_key_aut_mac(key_id_array, 1, (uint8_t *)cmd_buf, mac_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("sign pass \n");
    }
    else {
        printf("sign error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_rsa_verify_signature(uint8_t *msg, uint8_t *e, uint8_t *n, uint32_t nbitlen, uint8_t *sign)
{
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_rsa_verify_t *cmd_rsa_verify = NULL;

    /*********** verify ***********/
    cmd_rsa_verify = (cmd_rsa_verify_t *)cmd_buf;
    cmd_rsa_verify->cmd_id = RSA_VERIFY_SIGNATURE;

    set_big_endian_4byte((uint8_t *)cmd_rsa_verify->data_ptr, (uint32_t)msg);
    set_big_endian_4byte((uint8_t *)cmd_rsa_verify->e_ptr, (uint32_t)e);
    set_big_endian_4byte((uint8_t *)cmd_rsa_verify->signature_ptr, (uint32_t)sign);
    set_big_endian_4byte((uint8_t *)cmd_rsa_verify->n_ptr, (uint32_t)n);
    set_big_endian_2byte((uint8_t *)cmd_rsa_verify->n_len, (uint32_t)nbitlen);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("verify pass \n");
    }
    else {
        printf("verify error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_rsa_set_crt_prikey_ptr(uint8_t *p, uint8_t *q, uint8_t *dp, uint8_t *dq, uint8_t *u, rsa_crt_pri_key_ptr_t *rsa_crt_pri_key)
{
    set_big_endian_4byte((uint8_t *)rsa_crt_pri_key->p_ptr, (uint32_t)p);
    set_big_endian_4byte((uint8_t *)rsa_crt_pri_key->q_ptr, (uint32_t)q);
    set_big_endian_4byte((uint8_t *)rsa_crt_pri_key->dp_ptr, (uint32_t)dp);
    set_big_endian_4byte((uint8_t *)rsa_crt_pri_key->dq_ptr, (uint32_t)dq);
    set_big_endian_4byte((uint8_t *)rsa_crt_pri_key->u_ptr, (uint32_t)u);
    return CMD_RETURN_SUCESS;
}

uint32_t cmd_rsa_generate_crt_key(uint16_t ebitlen, uint16_t nbitlen, cmd_key_type_e generate_key_type, uint16_t generate_key_id,
                                  uint8_t *n, uint8_t *e, rsa_crt_pri_key_ptr_t *rsa_crt_pri_key, uint8_t *mac_buf)
{
    uint16_t key_id_array[2];
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_rsa_generateCRTkey_t *cmd_rsa_get_crt_key = NULL;

    /*********** generate key pair ***********/
    cmd_rsa_get_crt_key = (cmd_rsa_generateCRTkey_t *)cmd_buf;
    cmd_rsa_get_crt_key->cmd_id = RSA_GENERATE_CRT_KEY;

    set_big_endian_2byte((uint8_t *)cmd_rsa_get_crt_key->e_len, (uint32_t)ebitlen);
    set_big_endian_2byte((uint8_t *)cmd_rsa_get_crt_key->n_len, (uint32_t)nbitlen);

    if (CMD_KEY_EXTERNAL_PLAINTEXT == generate_key_type) {
        key_id_array[0] = 0;
    }
    else if (CMD_KEY_EXTERNAL_CIPHERTEXT == generate_key_type) {
        key_id_array[0] = generate_key_id;
    }

    set_big_endian_2byte((uint8_t *)cmd_rsa_get_crt_key->key_id, key_id_array[0]);

    set_big_endian_4byte((uint8_t *)cmd_rsa_get_crt_key->e_ptr, (uint32_t)e);
    set_big_endian_4byte((uint8_t *)cmd_rsa_get_crt_key->n_ptr, (uint32_t)n);
    set_big_endian_4byte((uint8_t *)cmd_rsa_get_crt_key->priKey_ptr, (uint32_t)rsa_crt_pri_key);

    set_big_endian_4byte((uint8_t *)cmd_rsa_get_crt_key->aut_info_ptr, (uint32_t)mac_buf);

    get_otp_key_aut_mac(key_id_array, 1, (uint8_t *)cmd_buf, mac_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("RSA_GENERATE_CRT_KEY eBitLen=%d, nBitLen=%d success \n", ebitlen, nbitlen);
    }
    else {
        printf("RSA_GENERATE_CRT_KEY eBitLen=%d, nBitLen=%d error, ret = 0x%x \n", ebitlen, nbitlen, ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_rsa_crt_decrypt(uint8_t *msg, uint8_t *e, uint8_t *n, uint32_t nbitlen, uint8_t *out, cmd_key_type_e decrypt_key_type, uint16_t decrypt_key_id, rsa_crt_pri_key_ptr_t *rsa_crt_pri_key, uint8_t *mac_buf)
{
    uint16_t key_id_array[2];
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_rsa_CRT_decrypt_t *cmd_rsa_crt_dec = NULL;

    /*********** decrypt ***********/
    cmd_rsa_crt_dec = (cmd_rsa_CRT_decrypt_t *)cmd_buf;
    cmd_rsa_crt_dec->cmd_id = RSA_CRT_DECRYPT;

    cmd_rsa_crt_dec->need_high_sec_ver = 1;

    set_big_endian_4byte((uint8_t *)cmd_rsa_crt_dec->data_ptr, (uint32_t)msg);
    set_big_endian_4byte((uint8_t *)cmd_rsa_crt_dec->e_ptr, (uint32_t)e);
    set_big_endian_4byte((uint8_t *)cmd_rsa_crt_dec->cipher_ptr, (uint32_t)out);
    set_big_endian_4byte((uint8_t *)cmd_rsa_crt_dec->priKey_ptr, (uint32_t)rsa_crt_pri_key);
    set_big_endian_2byte((uint8_t *)cmd_rsa_crt_dec->n_len, (uint32_t)nbitlen);

    if (CMD_KEY_EXTERNAL_PLAINTEXT == decrypt_key_type) {
        key_id_array[0] = 0;
    }
    else if (CMD_KEY_EXTERNAL_CIPHERTEXT == decrypt_key_type) {
        key_id_array[0] = decrypt_key_id;
    }
    else {
        return CMD_RETURN_FAIL;
    }

    set_big_endian_2byte((uint8_t *)cmd_rsa_crt_dec->key_id, key_id_array[0]);
    set_big_endian_4byte((uint8_t *)cmd_rsa_crt_dec->aut_info_ptr, (uint32_t)mac_buf);

    get_otp_key_aut_mac(key_id_array, 1, (uint8_t *)cmd_buf, mac_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("rsa crt dec pass \n");
    }
    else {
        printf("rsa crt dec error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_rsa_crt_generate_signature(uint8_t *msg, uint8_t *e, uint8_t *n, uint32_t nbitlen, uint8_t *out, cmd_key_type_e sign_key_type, uint16_t sign_key_id, rsa_crt_pri_key_ptr_t *rsa_crt_pri_key, uint8_t *mac_buf)
{
    uint16_t key_id_array[2];
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_rsa_CRT_generate_signature_t *cmd_rsa_crt_sign = NULL;

    /*********** sign ***********/
    cmd_rsa_crt_sign = (cmd_rsa_CRT_generate_signature_t *)cmd_buf;
    cmd_rsa_crt_sign->cmd_id = RSA_CRT_GENERATE_SIGNATURE;
    cmd_rsa_crt_sign->need_high_sec_ver = 1;

    set_big_endian_4byte((uint8_t *)cmd_rsa_crt_sign->data_ptr, (uint32_t)msg);
    set_big_endian_4byte((uint8_t *)cmd_rsa_crt_sign->e_ptr, (uint32_t)e);
    set_big_endian_4byte((uint8_t *)cmd_rsa_crt_sign->signature_ptr, (uint32_t)out);
    set_big_endian_4byte((uint8_t *)cmd_rsa_crt_sign->priKey_ptr, (uint32_t)rsa_crt_pri_key);
    set_big_endian_2byte((uint8_t *)cmd_rsa_crt_sign->n_len, (uint32_t)nbitlen);

    if (CMD_KEY_EXTERNAL_PLAINTEXT == sign_key_type) {
        key_id_array[0] = 0;
    }
    else if (CMD_KEY_EXTERNAL_CIPHERTEXT == sign_key_type) {
        key_id_array[0] = sign_key_id;
    }
    else {
        return CMD_RETURN_FAIL;
    }

    set_big_endian_2byte((uint8_t *)cmd_rsa_crt_sign->key_id, key_id_array[0]);
    set_big_endian_4byte((uint8_t *)cmd_rsa_crt_sign->aut_info_ptr, (uint32_t)mac_buf);

    get_otp_key_aut_mac(key_id_array, 1, (uint8_t *)cmd_buf, mac_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("rsa crt sign pass \n");
    }
    else {
        printf("rsa crt sign error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}