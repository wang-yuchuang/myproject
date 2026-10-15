/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <driver/taishan/crypto/osr_crypto/mailbox/cmd_eccp.h>

uint32_t cmd_eccp_point_doubling(uint8_t curve_id, uint8_t *G_x_y, uint8_t *output_x_y)
{
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_eccp_pointdoubling_t *cmd_eccp_dbl = NULL;

    /*********** point dbl ***********/
    cmd_eccp_dbl = (cmd_eccp_pointdoubling_t *)cmd_buf;
    cmd_eccp_dbl->cmd_id = ECCP_POINT_DOUBLING;

    cmd_eccp_dbl->curve_id = curve_id;
    set_big_endian_4byte((uint8_t *)cmd_eccp_dbl->inputPoint_ptr, (uint32_t)G_x_y);
    set_big_endian_4byte((uint8_t *)cmd_eccp_dbl->outputPoint_ptr, (uint32_t)output_x_y);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("ECCP_POINT_DOUBLING success \n");
    }
    else {
        printf("ECCP_POINT_DOUBLING error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_eccp_point_addition(uint8_t curve_id, uint8_t *kG_x_y, uint8_t *dblG_x_y, uint8_t *output_x_y)
{
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_eccp_pointaddition_t *cmd_eccp_add = NULL;

    /*********** point add ***********/
    cmd_eccp_add = (cmd_eccp_pointaddition_t *)cmd_buf;
    cmd_eccp_add->cmd_id = ECCP_POINT_ADDITION;

    cmd_eccp_add->curve_id = curve_id;
    set_big_endian_4byte((uint8_t *)cmd_eccp_add->inputPoint1_ptr, (uint32_t)kG_x_y);
    set_big_endian_4byte((uint8_t *)cmd_eccp_add->inputPoint2_ptr, (uint32_t)dblG_x_y);
    set_big_endian_4byte((uint8_t *)cmd_eccp_add->outputPoint_ptr, (uint32_t)output_x_y);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("ECCP_POINT_ADDITION success \n");
    }
    else {
        printf("ECCP_POINT_ADDITION error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_eccp_point_multiplication(uint8_t curve_id, uint8_t *G_x_y, uint8_t *k, uint8_t *output_x_y)
{
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_eccp_pointmultiplication_t *cmd_eccp_mul = NULL;

    /*********** point mul ***********/

    cmd_eccp_mul = (cmd_eccp_pointmultiplication_t *)cmd_buf;
    cmd_eccp_mul->cmd_id = ECCP_POINT_MULTIPLICATION;

    cmd_eccp_mul->curve_id = curve_id;
    set_big_endian_4byte((uint8_t *)cmd_eccp_mul->inputScalar_ptr, (uint32_t)k);
    set_big_endian_4byte((uint8_t *)cmd_eccp_mul->inputPoint_ptr, (uint32_t)G_x_y);
    set_big_endian_4byte((uint8_t *)cmd_eccp_mul->outputPoint_ptr, (uint32_t)output_x_y);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("ECCP_POINT_MULTIPLICATION success \n");
    }
    else {
        printf("ECCP_POINT_MULTIPLICATION error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_eccp_point_verify(uint8_t curve_id, uint8_t *G_x_y)
{
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_eccp_pointverifying_t *cmd_eccp_verify = NULL;

    /*********** point verify ***********/
    cmd_eccp_verify = (cmd_eccp_pointverifying_t *)cmd_buf;
    cmd_eccp_verify->cmd_id = ECCP_POINT_VERIFY;

    cmd_eccp_verify->curve_id = curve_id;
    set_big_endian_4byte((uint8_t *)cmd_eccp_verify->inputPoint_ptr, (uint32_t)G_x_y);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("ECCP_POINT_VERIFY success \n");
    }
    else {
        printf("ECCP_POINT_VERIFY error , ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_eccp_ecdh_generate_keypair(uint8_t curve_id, uint8_t *prikey, uint8_t *pubkey, cmd_key_type_e generate_key_type, uint16_t generate_key_id, uint8_t *mac_buf)
{
    uint16_t key_id_array[2];
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_eccp_generatekey_t *cmd_eccp_get_key = NULL;

    /*********** get key pair ***********/
    cmd_eccp_get_key = (cmd_eccp_generatekey_t *)cmd_buf;
    cmd_eccp_get_key->cmd_id = ECCP_GENERATE_KEY;

    if (CMD_KEY_EXTERNAL_PLAINTEXT == generate_key_type) {
        key_id_array[0] = 0;
    }
    else if (CMD_KEY_EXTERNAL_CIPHERTEXT == generate_key_type) {
        key_id_array[0] = generate_key_id;
    }
    else{
        key_id_array[0] = 0;
    }

    set_big_endian_2byte((uint8_t *)cmd_eccp_get_key->key_id, key_id_array[0]);

    cmd_eccp_get_key->curve_id = curve_id;
    set_big_endian_4byte((uint8_t *)cmd_eccp_get_key->pubKey_ptr, (uint32_t)pubkey);
    set_big_endian_4byte((uint8_t *)cmd_eccp_get_key->priKey_ptr, (uint32_t)prikey);
    set_big_endian_4byte((uint8_t *)cmd_eccp_get_key->aut_info_ptr, (uint32_t)mac_buf);

    get_otp_key_aut_mac(key_id_array, 1, (uint8_t *)cmd_buf, mac_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("ECCP_GENERATE_KEY success \n");
    }
    else {
        printf("ECCP_GENERATE_KEY error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_eccp_ecdh_exchange_key(uint8_t curve_id, uint8_t key_len, uint8_t *KA, uint8_t *pubkey_B, uint8_t *prikey_A, cmd_key_type_e ecdh_key_type, uint16_t ecdh_key_id, cmd_key_type_e output_key_type, uint16_t output_key_id, uint8_t *mac_buf)
{
    uint16_t key_id_array[3];
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_ecdh_exchangekey_t *cmd_eccp_exc_key = NULL;

    /*********** calc ***********/
    cmd_eccp_exc_key = (cmd_ecdh_exchangekey_t *)cmd_buf;
    cmd_eccp_exc_key->cmd_id = ECDH_EXCHANGE_KEY;

    cmd_eccp_exc_key->curve_id = curve_id;
    set_big_endian_4byte((uint8_t *)cmd_eccp_exc_key->PeerPubKey_ptr, (uint32_t)pubkey_B);
    set_big_endian_4byte((uint8_t *)cmd_eccp_exc_key->aut_info_ptr, (uint32_t)mac_buf);

    if (CMD_KEY_EXTERNAL_PLAINTEXT == ecdh_key_type) {
        set_big_endian_4byte((uint8_t *)cmd_eccp_exc_key->selfPrikey_ptr, (uint32_t)prikey_A);
        key_id_array[0] = 0;
        key_id_array[1] = 0;
    }
    else if (CMD_KEY_EXTERNAL_CIPHERTEXT == ecdh_key_type) {
        set_big_endian_4byte((uint8_t *)cmd_eccp_exc_key->selfPrikey_ptr, (uint32_t)prikey_A);
        key_id_array[0] = 0;
        key_id_array[1] = ecdh_key_id;
    }
    else if (CMD_KEY_INTERNAL == ecdh_key_type) {
        set_big_endian_4byte((uint8_t *)cmd_eccp_exc_key->selfPrikey_ptr, 0);
        key_id_array[0] = ecdh_key_id;
        key_id_array[1] = 0;
    }
    else {
        return CMD_RETURN_FAIL;
    }

    set_big_endian_2byte((uint8_t *)cmd_eccp_exc_key->key_id0, key_id_array[0]);
    set_big_endian_2byte((uint8_t *)cmd_eccp_exc_key->key_id1, key_id_array[1]);

    cmd_eccp_exc_key->key_len = key_len;
    set_big_endian_4byte((uint8_t *)cmd_eccp_exc_key->key_ptr, (uint32_t)KA);

    if (CMD_KEY_EXTERNAL_PLAINTEXT == output_key_type) {
        key_id_array[2] = 0;
    }
    else if (CMD_KEY_EXTERNAL_CIPHERTEXT == output_key_type) {
        key_id_array[2] = output_key_id;
    }else{
        key_id_array[2] = 0;
    }

    set_big_endian_2byte((uint8_t *)cmd_eccp_exc_key->key_id2, key_id_array[2]);

    uint32_clear((uint32_t *)mac_buf, 4 * 2);
    get_otp_key_aut_mac(key_id_array, 3, (uint8_t *)cmd_buf, mac_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {

    }
    else {
        printf("calc error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_eccp_ecdsa_gen_signature(uint8_t curve_id, uint8_t *e, uint32_t e_len, cmd_key_type_e sign_key_type, uint16_t sign_key_id, uint8_t *prikey, uint8_t *signature, uint8_t *mac_buf)
{
    uint16_t key_id_array[2];
    uint32_t cmd_buf[8];
    uint32_t i;
    uint32_t ret = 0;
    cmd_ecdsa_generatesignature_t *cmd_ecdsa_sign = NULL;

    /*********** ecdsa sign ***********/
    cmd_ecdsa_sign = (cmd_ecdsa_generatesignature_t *)cmd_buf;
    cmd_ecdsa_sign->cmd_id = ECDSA_GENERATE_SIGNATURE;

    cmd_ecdsa_sign->curve_id = curve_id;
    cmd_ecdsa_sign->e_len = e_len;
    set_big_endian_4byte((uint8_t *)cmd_ecdsa_sign->e_ptr, (uint32_t)e);
    set_big_endian_4byte((uint8_t *)cmd_ecdsa_sign->aut_info_ptr, (uint32_t)mac_buf);

    if (CMD_KEY_EXTERNAL_PLAINTEXT == sign_key_type) {
        key_id_array[0] = 0;
        key_id_array[1] = 0;
        set_big_endian_4byte((uint8_t *)cmd_ecdsa_sign->priKey_ptr, (uint32_t)prikey);
    }
    else if (CMD_KEY_EXTERNAL_CIPHERTEXT == sign_key_type) {
        key_id_array[0] = 0;
        key_id_array[1] = sign_key_id;
        set_big_endian_4byte((uint8_t *)cmd_ecdsa_sign->priKey_ptr, (uint32_t)prikey);
    }
    else if (CMD_KEY_INTERNAL == sign_key_type) {
        set_big_endian_4byte((uint8_t *)cmd_ecdsa_sign->priKey_ptr, 0);
        key_id_array[0] = sign_key_id;
        key_id_array[1] = 0;
    }
    else {
        return CMD_RETURN_FAIL;
    }

    set_big_endian_2byte((uint8_t *)cmd_ecdsa_sign->key_id0, key_id_array[0]);
    set_big_endian_2byte((uint8_t *)cmd_ecdsa_sign->key_id1, key_id_array[1]);
    set_big_endian_4byte((uint8_t *)cmd_ecdsa_sign->signature_ptr, (uint32_t)signature);

    get_otp_key_aut_mac(key_id_array, 2, (uint8_t *)cmd_buf, mac_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("ecdsa sign pass \n");
    }
    else {
        printf("ecdsa sign error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}


uint32_t cmd_eccp_ecdsa_verify_signature(uint8_t curve_id, uint8_t *e, uint32_t e_len, uint8_t *pubkey, uint8_t *signature)
{
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_ecdsa_verifysignature_t *cmd_ecdsa_verify = NULL;

    /*********** ecdsa verify ***********/
    cmd_ecdsa_verify = (cmd_ecdsa_verifysignature_t *)cmd_buf;
    cmd_ecdsa_verify->cmd_id = ECDSA_VERIFY_SIGNATURE;

    cmd_ecdsa_verify->curve_id = curve_id;
    cmd_ecdsa_verify->e_len = e_len;
    set_big_endian_4byte((uint8_t *)cmd_ecdsa_verify->e_ptr, (uint32_t)e);

    set_big_endian_4byte((uint8_t *)cmd_ecdsa_verify->pubKey_ptr, (uint32_t)pubkey);

    set_big_endian_4byte((uint8_t *)cmd_ecdsa_verify->signature_ptr, (uint32_t)signature);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("ecdsa verify pass \n");
    }
    else {
        printf("ecdsa verify error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}