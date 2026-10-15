/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <driver/taishan/crypto/osr_crypto/mailbox/cmd_sm2.h>

uint32_t cmd_sm2_get_z(uint8_t *id, uint32_t id_len, uint8_t *pubkey, uint8_t *z_buf)
{
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_sm2_getZ_t *cmd_sm2_get_z = NULL;

    /*********** generate Z value ***********/
    cmd_sm2_get_z = (cmd_sm2_getZ_t *)cmd_buf;
    cmd_sm2_get_z->cmd_id = SM2_GET_Z;

    set_big_endian_4byte((uint8_t *)cmd_sm2_get_z->id_ptr, (uint32_t)id);
    set_big_endian_2byte((uint8_t *)cmd_sm2_get_z->id_len, id_len);
    set_big_endian_4byte((uint8_t *)cmd_sm2_get_z->pubKey_ptr, (uint32_t)(pubkey));
    set_big_endian_4byte((uint8_t *)cmd_sm2_get_z->z, (uint32_t)z_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("SM2_GET_Z success \n");
    }
    else {
        print_buf_U8(id, id_len, "id");
        print_buf_U8(pubkey, 64, "pubkey");
        print_buf_U8(z_buf, 32, "z");
        printf("SM2_GET_Z error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_sm2_get_e(uint8_t *msg, uint32_t msg_len, uint8_t *z_buf, uint8_t *e_buf)
{
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_sm2_getE_t *cmd_sm2_get_e = NULL;

    /*********** generate Z value ***********/
    cmd_sm2_get_e = (cmd_sm2_getE_t *)cmd_buf;
    cmd_sm2_get_e->cmd_id = SM2_GET_E;

    set_big_endian_4byte((uint8_t *)cmd_sm2_get_e->data_ptr, (uint32_t)msg);
    set_big_endian_4byte((uint8_t *)cmd_sm2_get_e->data_len, msg_len);
    set_big_endian_4byte((uint8_t *)cmd_sm2_get_e->z, (uint32_t)z_buf);
    set_big_endian_4byte((uint8_t *)cmd_sm2_get_e->e, (uint32_t)e_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("SM2_GET_E success \n");
    }
    else {
        print_buf_U8(z_buf, 32, "z");
        print_buf_U8(msg, msg_len, "msg");
        printf("SM2_GET_E error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_sm2_get_pubkey_from_prikey(uint8_t *prikey, uint8_t *pubkey)
{
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_sm2_get_pubkey_from_prikey_t *cmd_gen_key = NULL;

    /*********** generate key pair ***********/
    cmd_gen_key = (cmd_sm2_get_pubkey_from_prikey_t *)cmd_buf;
    cmd_gen_key->cmd_id = SM2_GET_PUB_KEY;

    set_big_endian_2byte((uint8_t *)cmd_gen_key->key_id0, 0x0);
    set_big_endian_2byte((uint8_t *)cmd_gen_key->key_id1, 0x0);

    set_big_endian_4byte((uint8_t *)cmd_gen_key->pubKey_ptr, (uint32_t)pubkey);
    uint32_clear((uint32_t *)pubkey, 16);
    set_big_endian_4byte((uint8_t *)cmd_gen_key->priKey_ptr, (uint32_t)prikey);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("sm2 get_pubkey success \n");
    }
    else{
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_sm2_generatekey(cmd_key_type_e generate_key_type,  uint16_t generate_key_id, uint8_t *prikey, uint8_t *pubkey, uint8_t *mac_buf)
{
    uint16_t key_id_array[2];
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_sm2_generatekey_t *cmd_gen_key = NULL;

    /*********** generate key pair ***********/
    cmd_gen_key = (cmd_sm2_generatekey_t *)cmd_buf;
    cmd_gen_key->cmd_id = SM2_GENERATE_KEY;

    if (CMD_KEY_EXTERNAL_PLAINTEXT == generate_key_type) {
        key_id_array[0] = 0;
    }
    else if (CMD_KEY_EXTERNAL_CIPHERTEXT == generate_key_type) {
        key_id_array[0] = generate_key_id;
    }else{
        key_id_array[0] = 0;
    }

    set_big_endian_2byte((uint8_t *)cmd_gen_key->key_id, key_id_array[0]);

    set_big_endian_4byte((uint8_t *)cmd_gen_key->pubKey_ptr, (uint32_t)pubkey);
    uint32_clear((uint32_t *)pubkey, 16);
    set_big_endian_4byte((uint8_t *)cmd_gen_key->priKey_ptr, (uint32_t)prikey);
    uint32_clear((uint32_t *)prikey, 8);
    set_big_endian_4byte((uint8_t *)cmd_gen_key->aut_info_ptr, (uint32_t)mac_buf);

    get_otp_key_aut_mac(key_id_array, 1, (uint8_t *)cmd_buf, mac_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("SM2_GENERATE_KEY success \n");
    }
    else {
        print_buf_U8(pubkey, 64, "pubkey");
        print_buf_U8(prikey, 32, "prikey");
        printf("SM2_GENERATE_KEY error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_sm2_sign(uint8_t *e, uint8_t *prikey, cmd_key_type_e generate_sign_key_type, uint16_t generate_sign_key_id, uint8_t *signature, uint8_t *mac_buf)
{
    uint16_t key_id_array[2];
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_sm2_generatesignature_t *cmd_gen_signature = NULL;

    /*********** generate signature ***********/
    cmd_gen_signature = (cmd_sm2_generatesignature_t *)cmd_buf;
    cmd_gen_signature->cmd_id = SM2_GENERATE_SIGNATURE;

    cmd_gen_signature->need_high_sec_ver = 0;

    set_big_endian_4byte((uint8_t *)cmd_gen_signature->e_ptr, (uint32_t)e);

    if (CMD_KEY_EXTERNAL_PLAINTEXT == generate_sign_key_type) {
        set_big_endian_4byte((uint8_t *)cmd_gen_signature->priKey_ptr, (uint32_t)prikey);
        key_id_array[0] = 0;
        key_id_array[1] = 0;
    }
    else if (CMD_KEY_EXTERNAL_CIPHERTEXT == generate_sign_key_type) {
        set_big_endian_4byte((uint8_t *)cmd_gen_signature->priKey_ptr, (uint32_t)prikey);
        key_id_array[0] = 0;
        key_id_array[1] = generate_sign_key_id;
    }
    else if (CMD_KEY_INTERNAL == generate_sign_key_type) {
        set_big_endian_4byte((uint8_t *)cmd_gen_signature->priKey_ptr, 0);
        key_id_array[0] = generate_sign_key_id;
        key_id_array[1] = 0;
    }

    set_big_endian_2byte((uint8_t *)cmd_gen_signature->key_id0, key_id_array[0]);
    set_big_endian_2byte((uint8_t *)cmd_gen_signature->key_id1, key_id_array[1]);
    set_big_endian_4byte((uint8_t *)cmd_gen_signature->signature_ptr, (uint32_t)signature);
    set_big_endian_4byte((uint8_t *)cmd_gen_signature->aut_info_ptr, (uint32_t)mac_buf);

    get_otp_key_aut_mac(key_id_array, 2, (uint8_t *)cmd_buf, mac_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if ((SEIP_SUCCESS == ret) && (((uint32_t *)signature)[0] != 0)) {
        printf("sign success \n");
    }
    else {
        print_buf_U8(e, 32, "e");
        print_buf_U8(prikey, 32, "prikey");
        print_buf_U8(signature, 64, "signature");
        printf("SM2_GENERATE_SIGNATURE error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_sm2_verify(uint8_t *e, uint8_t *pubkey, uint8_t *signature)
{
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_sm2_verifysignature_t *cmd_verify_signature = NULL;

    /*********** verify signature ***********/
    cmd_verify_signature = (cmd_sm2_verifysignature_t *)cmd_buf;
    cmd_verify_signature->cmd_id = SM2_VERIFY_SIGNATURE;

    cmd_verify_signature->need_high_sec_ver = 0;

    set_big_endian_4byte((uint8_t *)cmd_verify_signature->e_ptr, (uint32_t)e);

    set_big_endian_4byte((uint8_t *)cmd_verify_signature->pubKey_ptr, (uint32_t)pubkey);

    set_big_endian_4byte((uint8_t *)cmd_verify_signature->signature_ptr, (uint32_t)signature);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("verify success \n");
    }
    else {
        print_buf_U8(e, 32, "e");
        print_buf_U8(pubkey, 64, "pubkey");
        print_buf_U8(signature, 64, "signature");
        printf("SM2_VERIFY error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_sm2_enc(uint8_t *msg, uint32_t msg_len, uint8_t *pubkey, uint8_t *cipher)
{
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_sm2_encrypt_t *cmd_encrypt = NULL;

    /*********** encryption ***********/
    cmd_encrypt = (cmd_sm2_encrypt_t *)cmd_buf;
    cmd_encrypt->cmd_id = SM2_ENCRYPT;

    cmd_encrypt->need_high_sec_ver = 0;

    set_big_endian_4byte((uint8_t *)cmd_encrypt->data_ptr, (uint32_t)msg);
    set_big_endian_4byte((uint8_t *)cmd_encrypt->data_len, (uint32_t)msg_len);

    set_big_endian_4byte((uint8_t *)cmd_encrypt->pubKey_ptr, (uint32_t)pubkey);
    set_big_endian_4byte((uint8_t *)cmd_encrypt->cipher_ptr, (uint32_t)cipher);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if ((SEIP_SUCCESS == ret) && (((uint32_t *)cipher)[0] != 0)) {

        printf("enc success \n");
    }
    else {
        print_buf_U8(msg, msg_len, "msg");
        print_buf_U8(pubkey, 64, "pubkey");
        print_buf_U8(cipher, msg_len + 97, "cipher");
        printf("SM2_ENCRYPT error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_sm2_dec(uint8_t *prikey,  uint8_t *msg, uint8_t *cipher, uint32_t cipher_len, cmd_key_type_e decrypt_key_type, uint16_t decrypt_key_id, uint8_t *mac_buf)
{
    uint16_t key_id_array[2];
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_sm2_decrypt_t *cmd_decrypt = NULL;

    /*********** decryption ***********/
    cmd_decrypt = (cmd_sm2_decrypt_t *)cmd_buf;
    cmd_decrypt->cmd_id = SM2_DECRYPT;

    cmd_decrypt->need_high_sec_ver = 0;

    set_big_endian_4byte((uint8_t *)cmd_decrypt->cipher_ptr, (uint32_t)cipher);
    set_big_endian_4byte((uint8_t *)cmd_decrypt->cipher_len, cipher_len);

    if (CMD_KEY_EXTERNAL_PLAINTEXT == decrypt_key_type) {
        set_big_endian_4byte((uint8_t *)cmd_decrypt->priKey_ptr, (uint32_t)prikey);
        key_id_array[0] = 0;
        key_id_array[1] = 0;
    }
    else if (CMD_KEY_EXTERNAL_CIPHERTEXT == decrypt_key_type) {
        set_big_endian_4byte((uint8_t *)cmd_decrypt->priKey_ptr, (uint32_t)prikey);
        key_id_array[0] = 0;
        key_id_array[1] = decrypt_key_id;
    }
    else if (CMD_KEY_INTERNAL == decrypt_key_type) {
        set_big_endian_4byte((uint8_t *)cmd_decrypt->priKey_ptr, 0);
        key_id_array[0] = decrypt_key_id;
        key_id_array[1] = 0;
    }

    set_big_endian_2byte((uint8_t *)cmd_decrypt->key_id0, key_id_array[0]);
    set_big_endian_2byte((uint8_t *)cmd_decrypt->key_id1, key_id_array[1]);
    set_big_endian_4byte((uint8_t *)cmd_decrypt->data_ptr, (uint32_t)msg);
    set_big_endian_4byte((uint8_t *)cmd_decrypt->aut_info_ptr, (uint32_t)mac_buf);

    get_otp_key_aut_mac(key_id_array, 2, (uint8_t *)cmd_buf, mac_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if ((SEIP_SUCCESS == ret) && (((uint32_t *)msg)[0] != 0)) {
        printf("dec success\n");
    }
    else {
        print_buf_U8(msg, cipher_len - 97, "msg");
        print_buf_U8(cipher, cipher_len, "cipher");
        print_buf_U8(prikey, 32, "prikey");

        printf("SM2_DECRYPT error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_sm2_set_exckey_input_ptr(uint8_t *dA, uint8_t *RA, uint8_t *rA, uint8_t *RB, uint8_t *PB, uint8_t *ZA, uint8_t *ZB, sm2_exc_key_input_ptr_t *cmd_sm2_exc_key_input)
{
    set_big_endian_4byte((uint8_t *)cmd_sm2_exc_key_input->dA_ptr, (uint32_t)dA);
    set_big_endian_4byte((uint8_t *)cmd_sm2_exc_key_input->RA_ptr, (uint32_t)RA);
    set_big_endian_4byte((uint8_t *)cmd_sm2_exc_key_input->rA_ptr, (uint32_t)rA);

    set_big_endian_4byte((uint8_t *)cmd_sm2_exc_key_input->RB_ptr, (uint32_t)RB);
    set_big_endian_4byte((uint8_t *)cmd_sm2_exc_key_input->PB_ptr, (uint32_t)PB);
    set_big_endian_4byte((uint8_t *)cmd_sm2_exc_key_input->ZA_ptr, (uint32_t)ZA);
    set_big_endian_4byte((uint8_t *)cmd_sm2_exc_key_input->ZB_ptr, (uint32_t)ZB);
    return CMD_RETURN_SUCESS;
}

uint32_t cmd_sm2_exchange_key(cmd_sm2_exchange_role_e role, cmd_sm2_exchangekey_keyinfo_t *cmd_sm2_exckey_keyinfo,
                              sm2_exc_key_input_ptr_t *cmd_sm2_exc_key_input, uint32_t kbytelen,
                              uint8_t *S1, uint8_t *SA, uint8_t *KA, uint8_t *mac_buf)
{
    uint16_t key_id_array[4];
    uint32_t cmd_buf[8];
    uint32_t ret = 0;
    cmd_sm2_exchangekey_t *cmd_sm2_exc_key = NULL;

    /*********** A side exchange key ***********/
    cmd_sm2_exc_key = (cmd_sm2_exchangekey_t *)cmd_buf;
    cmd_sm2_exc_key->cmd_id = SM2_EXCHANGE_KEY;

    cmd_sm2_exc_key->need_high_sec_ver = 0;

    cmd_sm2_exc_key->role = role;

    if (CMD_KEY_EXTERNAL_PLAINTEXT == cmd_sm2_exckey_keyinfo->dA_key_type) {
        key_id_array[0] = 0;
        key_id_array[2] = 0;
    }
    else if (CMD_KEY_EXTERNAL_CIPHERTEXT == cmd_sm2_exckey_keyinfo->dA_key_type) {
        key_id_array[0] = 0;
        key_id_array[2] = cmd_sm2_exckey_keyinfo->enc_key_id;

        if (CMD_KEY_EXTERNAL_CIPHERTEXT != cmd_sm2_exckey_keyinfo->rA_key_type) {
            return CMD_RETURN_FAIL;
        }
    }
    else if (CMD_KEY_INTERNAL == cmd_sm2_exckey_keyinfo->dA_key_type) {
        key_id_array[0] = cmd_sm2_exckey_keyinfo->dA_key_id;
        key_id_array[2] = 0;
    }

    set_big_endian_2byte((uint8_t *)cmd_sm2_exc_key->key_id0, key_id_array[0]);
    set_big_endian_2byte((uint8_t *)cmd_sm2_exc_key->key_id2, key_id_array[2]);

    if (CMD_KEY_EXTERNAL_PLAINTEXT == cmd_sm2_exckey_keyinfo->rA_key_type) {
        key_id_array[1] = 0;
        key_id_array[2] = 0;
    }
    else if (CMD_KEY_EXTERNAL_CIPHERTEXT == cmd_sm2_exckey_keyinfo->rA_key_type) {
        key_id_array[1] = 0;
        key_id_array[2] = cmd_sm2_exckey_keyinfo->enc_key_id;
    }
    else if (CMD_KEY_INTERNAL == cmd_sm2_exckey_keyinfo->rA_key_type) {
        key_id_array[1] = cmd_sm2_exckey_keyinfo->rA_key_id;
        key_id_array[2] = 0;
    }

    set_big_endian_2byte((uint8_t *)cmd_sm2_exc_key->key_id1, key_id_array[1]);
    set_big_endian_2byte((uint8_t *)cmd_sm2_exc_key->key_id2, key_id_array[2]);

    set_big_endian_4byte((uint8_t *)cmd_sm2_exc_key->input_ptr, (uint32_t)cmd_sm2_exc_key_input);
    cmd_sm2_exc_key->key_len = kbytelen;

    if (CMD_KEY_EXTERNAL_PLAINTEXT == cmd_sm2_exckey_keyinfo->output_key_type) {
        key_id_array[3] = 0;
    }
    else if (CMD_KEY_EXTERNAL_CIPHERTEXT == cmd_sm2_exckey_keyinfo->output_key_type) {
        key_id_array[3] = cmd_sm2_exckey_keyinfo->output_key_id;
    }

    set_big_endian_2byte((uint8_t *)cmd_sm2_exc_key->key_id3, key_id_array[3]);

    set_big_endian_4byte((uint8_t *)cmd_sm2_exc_key->outputKey_ptr, (uint32_t)KA);
    set_big_endian_4byte((uint8_t *)cmd_sm2_exc_key->s1_s2_ptr, (uint32_t)S1);
    set_big_endian_4byte((uint8_t *)cmd_sm2_exc_key->sa_sb_ptr, (uint32_t)SA);
    set_big_endian_4byte((uint8_t *)cmd_sm2_exc_key->aut_info_ptr, (uint32_t)mac_buf);

    get_otp_key_aut_mac(key_id_array, 4, (uint8_t *)cmd_buf, mac_buf);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if ((SEIP_SUCCESS == ret)) {
        printf("exckey success \n");
    }
    else {
        print_buf_U8(KA, kbytelen, "KA");
        print_buf_U8(S1, 32, "S1");
        print_buf_U8(SA, 32, "SA");
        printf("sponsor error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}