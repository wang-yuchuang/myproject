/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <driver/taishan/crypto/osr_crypto/mailbox/cmd_sm9.h>

uint8_t cmd_sm9_sign_gen_master_key(uint8_t *ks, uint8_t *pub_s)
{
    uint8_t ret = 0;
    uint32_t cmd_buf[8];
    cmd_sm9_sign_generate_master_key_t *cmd_gen_mast_key = NULL;

    /*********** generate master key pair ***********/
    cmd_gen_mast_key = (cmd_sm9_sign_generate_master_key_t *)cmd_buf;
    cmd_gen_mast_key->cmd_id = SM9_SIGN_GENERATE_MASTER_KEY;

    set_big_endian_4byte((uint8_t *)cmd_gen_mast_key->mastPubKey_ptr, (uint32_t)pub_s);
    set_big_endian_4byte((uint8_t *)cmd_gen_mast_key->mastPriKey_ptr, (uint32_t)ks);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("SM9_SIGN_GENERATE_MASTER_KEY success \n");
    }
    else {
        print_buf_U8(ks, 32, "ks");
        print_buf_U8(pub_s, 128, "pub_s");
        printf("SM9_SIGN_GENERATE_MASTER_KEY error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint8_t cmd_sm9_sign_gen_user_prikey(uint8_t *ida, uint32_t ida_len, uint8_t *ks, uint8_t *dsa)
{
    uint8_t ret = 0;
    uint32_t cmd_buf[8];
    cmd_sm9_sign_generate_user_prikey_t *cmd_gen_user_key = NULL;

    /*********** generate user private key ***********/
    cmd_gen_user_key = (cmd_sm9_sign_generate_user_prikey_t *)cmd_buf;
    cmd_gen_user_key->cmd_id = SM9_SIGN_GENERATE_USER_PRIVATE_KEY;

    set_big_endian_4byte((uint8_t *)cmd_gen_user_key->id_ptr, (uint32_t)ida);
    set_big_endian_2byte((uint8_t *)cmd_gen_user_key->id_len, ida_len);

    cmd_gen_user_key->hid = 0x1;
    set_big_endian_4byte((uint8_t *)cmd_gen_user_key->mastPriKey_ptr, (uint32_t)ks);
    set_big_endian_4byte((uint8_t *)cmd_gen_user_key->userPriKey_ptr, (uint32_t)dsa);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("gen user pri key success ida_len=%d \n", ida_len);
    }
    else {
        print_buf_U8(ida, ida_len, "ida");
        print_buf_U8(dsa, 64, "dsa");
        printf("SM9_SIGN_GENERATE_USER_PRIVATE_KEY error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint8_t cmd_sm9_gen_signature(uint8_t *msg, uint32_t msg_len, uint8_t *pub_s, uint8_t *dsa, uint8_t *signature)
{
    uint8_t ret = 0;
    uint32_t cmd_buf[8];
    cmd_sm9_generatesignature_t *cmd_gen_signature = NULL;

    /*********** generate signature ***********/
    cmd_gen_signature = (cmd_sm9_generatesignature_t *)cmd_buf;
    cmd_gen_signature->cmd_id = SM9_GENERATE_SIGNATURE;

    set_big_endian_4byte((uint8_t *)cmd_gen_signature->data_ptr, (uint32_t)msg);
    set_big_endian_4byte((uint8_t *)cmd_gen_signature->data_len, msg_len);

    set_big_endian_4byte((uint8_t *)cmd_gen_signature->mastPubKey_ptr, (uint32_t)pub_s);
    set_big_endian_4byte((uint8_t *)cmd_gen_signature->userPriKey_ptr, (uint32_t)dsa);
    set_big_endian_4byte((uint8_t *)cmd_gen_signature->signature_ptr, (uint32_t)signature);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if ((SEIP_SUCCESS == ret) && (((uint32_t *)signature)[0] != 0)) {
        printf("sign success msg_len=%d \n", msg_len);
    }
    else {
        print_buf_U8(msg, msg_len, "msg");
        print_buf_U8(pub_s, 128, "Ppub_s");
        print_buf_U8(dsa, 64, "dsa");
        print_buf_U8(signature, 32 + 65, "signature");
        printf("SM9_GENERATE_SIGNATURE error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint8_t cmd_sm9_verify_signature(uint8_t *msg, uint32_t msg_len, uint8_t *ida, uint32_t ida_len, uint8_t *pub_s, uint8_t *signature)
{
    uint8_t ret = 0;
    uint32_t cmd_buf[8];
    cmd_sm9_verifysignature_t *cmd_verify_signature = NULL;

    /*********** verify signature ***********/
    cmd_verify_signature = (cmd_sm9_verifysignature_t *)cmd_buf;
    cmd_verify_signature->cmd_id = SM9_VERIFY_SIGNATURE;

    set_big_endian_4byte((uint8_t *)cmd_verify_signature->data_ptr, (uint32_t)msg);
    set_big_endian_4byte((uint8_t *)cmd_verify_signature->data_len, msg_len);
    set_big_endian_4byte((uint8_t *)cmd_verify_signature->id_ptr, (uint32_t)ida);
    set_big_endian_2byte((uint8_t *)cmd_verify_signature->id_len, ida_len);
    cmd_verify_signature->hid = 0x1;
    set_big_endian_4byte((uint8_t *)cmd_verify_signature->mastPubKey_ptr, (uint32_t)pub_s);
    set_big_endian_4byte((uint8_t *)cmd_verify_signature->signature_ptr, (uint32_t)signature);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("verify success ida_len=%d, msg_len=%d \n", ida_len, msg_len);
    }
    else {
        print_buf_U8(ida, ida_len, "ida");
        print_buf_U8(msg, msg_len, "msg");
        print_buf_U8(pub_s, 128, "Ppub_s");
        print_buf_U8(signature, 32 + 65, "signature");
        printf("SM9_VERIFY_SIGNATURE 1 error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint8_t cmd_sm9_enc_gen_master_key(uint8_t *ke, uint8_t *pub_e)
{
    uint8_t ret = 0;
    uint32_t cmd_buf[8];
    cmd_sm9_enc_generate_master_key_t *cmd_gen_mast_key = NULL;

    /*********** generate master key pair ***********/

    cmd_gen_mast_key = (cmd_sm9_enc_generate_master_key_t *)cmd_buf;
    cmd_gen_mast_key->cmd_id = SM9_ENC_GENERATE_MASTER_KEY;

    set_big_endian_4byte((uint8_t *)cmd_gen_mast_key->mastPubKey_ptr, (uint32_t)pub_e);
    set_big_endian_4byte((uint8_t *)cmd_gen_mast_key->mastPriKey_ptr, (uint32_t)ke);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("SM9_ENC_GENERATE_MASTER_KEY success \n");
    }
    else {
        print_buf_U8(ke, 32, "ke");
        print_buf_U8(pub_e, 64, "Ppub_e");
        printf("SM9_ENC_GENERATE_MASTER_KEY error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint8_t cmd_sm9_enc_gen_user_prikey(uint8_t *idb, uint32_t idb_len, uint8_t *ke, uint8_t *deb)
{
    uint8_t ret = 0;
    uint32_t cmd_buf[8];
    cmd_sm9_enc_generate_user_prikey_t *cmd_gen_user_key = NULL;

    /*********** generate user private key ***********/

    cmd_gen_user_key = (cmd_sm9_enc_generate_user_prikey_t *)cmd_buf;
    cmd_gen_user_key->cmd_id = SM9_ENC_GENERATE_USER_PRIVATE_KEY;

    set_big_endian_4byte((uint8_t *)cmd_gen_user_key->id_ptr, (uint32_t)idb);
    set_big_endian_2byte((uint8_t *)cmd_gen_user_key->id_len, idb_len);

    cmd_gen_user_key->hid = 0x3;
    set_big_endian_4byte((uint8_t *)cmd_gen_user_key->mastPriKey_ptr, (uint32_t)ke);
    set_big_endian_4byte((uint8_t *)cmd_gen_user_key->userPriKey_ptr, (uint32_t)deb);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("gen user pri key success idb_len=%d \n", idb_len);
    }
    else {
        print_buf_U8(idb, idb_len, "idb");
        print_buf_U8(ke, 32, "ke");
        print_buf_U8(deb, 128, "deb");
        printf("SM9_SIGN_GENERATE_USER_PRIVATE_KEY error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint8_t cmd_sm9_enc_wrap_key(uint8_t *idb, uint32_t idb_len, uint8_t *pub_e, uint8_t *K, uint32_t kbytelen, uint8_t *C)
{
    uint8_t ret = 0;
    uint32_t cmd_buf[8];
    cmd_sm9_wrapkey_t *cmd_wrap_key = NULL;

    /*********** wrap key ***********/

    cmd_wrap_key = (cmd_sm9_wrapkey_t *)cmd_buf;
    cmd_wrap_key->cmd_id = SM9_WRAP_KEY;

    set_big_endian_4byte((uint8_t *)cmd_wrap_key->id_ptr, (uint32_t)idb);
    set_big_endian_2byte((uint8_t *)cmd_wrap_key->id_len, idb_len);

    cmd_wrap_key->hid = 0x3;
    set_big_endian_4byte((uint8_t *)cmd_wrap_key->mastPubKey_ptr, (uint32_t)pub_e);
    set_big_endian_4byte((uint8_t *)cmd_wrap_key->key_ptr, (uint32_t)K);
    cmd_wrap_key->key_len = kbytelen;
    set_big_endian_4byte((uint8_t *)cmd_wrap_key->cipher_ptr, (uint32_t)C);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("wrap success idb_len=%d, key_len=%d \n", idb_len, kbytelen);
    }
    else {
        print_buf_U8(idb, idb_len, "idb");
        print_buf_U8(pub_e, 64, "pub_e");
        print_buf_U8(K, kbytelen, "K");
        print_buf_U8(C, 64, "C");
        printf("SM9_WRAP_KEY error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint8_t cmd_sm9_enc_unwrap_key(uint8_t *idb, uint32_t idb_len, uint8_t *deb, uint8_t *K_unwrap, uint32_t kbytelen, uint8_t *C)
{
    uint8_t ret = 0;
    uint32_t cmd_buf[8];
    cmd_sm9_unwrapkey_t *cmd_unwrap_key = NULL;

    /*********** unwrap key ***********/

    cmd_unwrap_key = (cmd_sm9_unwrapkey_t *)cmd_buf;
    cmd_unwrap_key->cmd_id = SM9_UNWRAP_KEY;

    set_big_endian_4byte((uint8_t *)cmd_unwrap_key->id_ptr, (uint32_t)idb);
    set_big_endian_2byte((uint8_t *)cmd_unwrap_key->id_len, idb_len);

    set_big_endian_4byte((uint8_t *)cmd_unwrap_key->userPriKey_ptr, (uint32_t)deb);
    set_big_endian_4byte((uint8_t *)cmd_unwrap_key->key_ptr, (uint32_t)K_unwrap);
    cmd_unwrap_key->key_len = kbytelen;
    set_big_endian_4byte((uint8_t *)cmd_unwrap_key->cipher_ptr, (uint32_t)C);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if ((SEIP_SUCCESS == ret) && (((uint32_t *)K_unwrap)[0] != 0)) {
        printf("unwrap success idb_len=%d, key_len=%d \n", idb_len, kbytelen);
    }
    else {
        print_buf_U8(idb, idb_len, "idb");
        print_buf_U8(deb, 128, "deb");
        print_buf_U8(C, 64, "C");
        print_buf_U8(K_unwrap, kbytelen, "K_unwrap");
        printf("SM9_UNWRAP_KEY error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint8_t cmd_sm9_enc_kdf_stream_cipher(uint8_t *idb, uint32_t idb_len, uint32_t key2_len, uint8_t *pub_e, uint8_t *msg, uint32_t msg_len, uint8_t *cipher, uint32_t *cipher_len)
{
    uint8_t ret = 0;
    uint32_t cmd_buf[8];
    cmd_sm9_encrypt_t *cmd_encrypt = NULL;

    /*********** encryption KDF ***********/

    cmd_encrypt = (cmd_sm9_encrypt_t *)cmd_buf;
    cmd_encrypt->cmd_id = SM9_ENCRYPT;

    set_big_endian_4byte((uint8_t *)cmd_encrypt->data_ptr, (uint32_t)msg);
    set_big_endian_4byte((uint8_t *)cmd_encrypt->data_len, msg_len);

    set_big_endian_4byte((uint8_t *)cmd_encrypt->id_ptr, (uint32_t)idb);
    set_big_endian_2byte((uint8_t *)cmd_encrypt->id_len, idb_len);

    cmd_encrypt->hid = 0x3;
    cmd_encrypt->enc_type = CMD_SM9_ENC_KDF_STREAM_CIPHER;

    cmd_encrypt->key2_len = key2_len;

    set_big_endian_4byte((uint8_t *)cmd_encrypt->mastPubKey_ptr, (uint32_t)pub_e);
    set_big_endian_4byte((uint8_t *)cmd_encrypt->cipher_ptr, (uint32_t)cipher);
    set_big_endian_4byte((uint8_t *)cmd_encrypt->cipherLen_ptr, (uint32_t)cipher_len);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if ((SEIP_SUCCESS == ret) && (((uint32_t *)cipher)[0] != 0)) {
        printf("enc KDF success idb_len=%d, msg_len=%d, key2_len=%d \n", idb_len, msg_len, key2_len);
    }
    else {
        print_buf_U8(msg, msg_len, "msg");
        print_buf_U8(idb, idb_len, "idb");
        print_buf_U8(pub_e, 64, "pub_e");
        print_buf_U8(cipher, cipher_len[0], "cipher");
        printf("SM9_ENCRYPT error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint8_t cmd_sm9_dec_kdf_stream_cipher(uint8_t *idb, uint32_t idb_len, uint32_t key2_len, uint8_t *deb, uint8_t *msg, uint32_t *msg_len, uint8_t *cipher, uint32_t cipher_len)
{
    uint8_t ret = 0;
    uint32_t cmd_buf[8];
    cmd_sm9_decrypt_t *cmd_decrypt = NULL;

    /*********** decryption KDF ***********/

    cmd_decrypt = (cmd_sm9_decrypt_t *)cmd_buf;
    cmd_decrypt->cmd_id = SM9_DECRYPT;

    set_big_endian_4byte((uint8_t *)cmd_decrypt->cipher_ptr, (uint32_t)cipher);
    set_big_endian_4byte((uint8_t *)cmd_decrypt->cipher_len, cipher_len);

    set_big_endian_4byte((uint8_t *)cmd_decrypt->id_ptr, (uint32_t)idb);
    set_big_endian_2byte((uint8_t *)cmd_decrypt->id_len, idb_len);

    cmd_decrypt->enc_type = CMD_SM9_ENC_KDF_STREAM_CIPHER;
    cmd_decrypt->key2_len = key2_len;

    set_big_endian_4byte((uint8_t *)cmd_decrypt->userPriKey_ptr, (uint32_t)deb);
    set_big_endian_4byte((uint8_t *)cmd_decrypt->data_ptr, (uint32_t)msg);
    set_big_endian_4byte((uint8_t *)cmd_decrypt->dataLen_ptr, (uint32_t)msg_len);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if ((SEIP_SUCCESS == ret) && (((uint32_t *)msg)[0] != 0)) {
        printf("dec KDF success idb_len=%d, msg_len=%d, key2_len=%d \n", idb_len, *msg_len, key2_len);
    }
    else {
        print_buf_U8(cipher, cipher_len, "cipher");
        print_buf_U8(idb, idb_len, "idb");
        print_buf_U8(deb, 128, "deb");
        print_buf_U8(msg, *msg_len, "msg recovered");
        printf("SM9_DECRYPT error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}


uint8_t cmd_sm9_enc_kdf_block_cipher(uint8_t *idb, uint32_t idb_len, uint32_t key2_len, uint8_t *pub_e, uint8_t *msg, uint32_t msg_len, uint8_t *cipher, uint32_t *cipher_len, uint8_t padding_type)
{
    uint8_t ret = 0;
    uint32_t cmd_buf[8];
    cmd_sm9_encrypt_t *cmd_encrypt = NULL;

    /*********** encryption BLOCK NO PADDING ***********/

    cmd_encrypt = (cmd_sm9_encrypt_t *)cmd_buf;
    cmd_encrypt->cmd_id = SM9_ENCRYPT;

    set_big_endian_4byte((uint8_t *)cmd_encrypt->data_ptr, (uint32_t)msg);
    set_big_endian_4byte((uint8_t *)cmd_encrypt->data_len, (uint32_t)msg_len);

    set_big_endian_4byte((uint8_t *)cmd_encrypt->id_ptr, (uint32_t)idb);
    set_big_endian_2byte((uint8_t *)cmd_encrypt->id_len, idb_len);

    cmd_encrypt->hid = 0x3;
    cmd_encrypt->enc_type = CMD_SM9_ENC_KDF_BLOCK_CIPHER;

    if (padding_type < CMD_SM9_ENC_MAX_PADDING) {
        cmd_encrypt->padding_type = padding_type;
    }

    cmd_encrypt->key2_len = key2_len;

    set_big_endian_4byte((uint8_t *)cmd_encrypt->mastPubKey_ptr, (uint32_t)pub_e);
    set_big_endian_4byte((uint8_t *)cmd_encrypt->cipher_ptr, (uint32_t)cipher);
    set_big_endian_4byte((uint8_t *)cmd_encrypt->cipherLen_ptr, (uint32_t)cipher_len);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if ((SEIP_SUCCESS == ret) && (((uint32_t *)cipher)[0] != 0)) {
        printf("enc BLOCK NO PADDING success idb_len=%d, msg_len=%d, key2_len=%d \n", idb_len, msg_len, key2_len);
    }
    else {
        print_buf_U8(msg, msg_len, "msg");
        print_buf_U8(idb, idb_len, "idb");
        print_buf_U8(pub_e, 64, "Ppub_e");
        print_buf_U8(cipher, cipher_len[0], "cipher");
        printf("SM9_ENCRYPT error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint8_t cmd_sm9_dec_kdf_block_cipher(uint8_t *id, uint32_t id_len, uint32_t key2_len, uint8_t *deb, uint8_t *msg, uint32_t *msg_len, uint8_t *cipher, uint32_t cipher_len, uint8_t padding_type)
{
    uint8_t ret = 0;
    uint32_t cmd_buf[8];
    cmd_sm9_decrypt_t *cmd_decrypt = NULL;

    /*********** decryption BLOCK NO PADDING ***********/

    cmd_decrypt = (cmd_sm9_decrypt_t *)cmd_buf;
    cmd_decrypt->cmd_id = SM9_DECRYPT;

    set_big_endian_4byte((uint8_t *)cmd_decrypt->cipher_ptr, (uint32_t)cipher);
    set_big_endian_4byte((uint8_t *)cmd_decrypt->cipher_len, cipher_len);

    set_big_endian_4byte((uint8_t *)cmd_decrypt->id_ptr, (uint32_t)id);
    set_big_endian_2byte((uint8_t *)cmd_decrypt->id_len, id_len);

    cmd_decrypt->enc_type = CMD_SM9_ENC_KDF_BLOCK_CIPHER;

    if (padding_type < CMD_SM9_ENC_MAX_PADDING) {
        cmd_decrypt->padding_type = padding_type;
    }

    cmd_decrypt->key2_len = key2_len;

    set_big_endian_4byte((uint8_t *)cmd_decrypt->userPriKey_ptr, (uint32_t)deb);
    set_big_endian_4byte((uint8_t *)cmd_decrypt->data_ptr, (uint32_t)msg);
    set_big_endian_4byte((uint8_t *)cmd_decrypt->dataLen_ptr, (uint32_t)msg_len);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if ((SEIP_SUCCESS == ret) && (((uint32_t *)msg)[0] != 0)) {
        printf("dec BLOCK NO PADDING success idb_len=%d, msg_len=%d, key2_len=%d \n", id_len, *msg_len, key2_len);
    }
    else {
        print_buf_U8(cipher, cipher_len, "cipher");
        print_buf_U8(id, id_len, "idb");
        print_buf_U8(deb, 128, "deb");
        print_buf_U8(msg, *msg_len, "msg recovered");
        printf("SM9_DECRYPT error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint8_t cmd_sm9_exckey_gen_master_key(uint8_t *ke, uint8_t *pub_e)
{
    uint8_t ret = 0;
    uint32_t cmd_buf[8];
    cmd_sm9_exckey_generate_master_key_t *cmd_gen_mast_key = NULL;

    /*********** generate master key pair ***********/

    cmd_gen_mast_key = (cmd_sm9_exckey_generate_master_key_t *)cmd_buf;
    cmd_gen_mast_key->cmd_id = SM9_EXCKEY_GENERATE_MASTER_KEY;

    set_big_endian_4byte((uint8_t *)cmd_gen_mast_key->mastPubKey_ptr, (uint32_t)pub_e);
    set_big_endian_4byte((uint8_t *)cmd_gen_mast_key->mastPriKey_ptr, (uint32_t)ke);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("SM9_EXCKEY_GENERATE_MASTER_KEY success \n");
    }
    else {
        print_buf_U8(ke, 32, "ke");
        print_buf_U8(pub_e, 64, "pub_e");
        printf("SM9_EXCKEY_GENERATE_MASTER_KEY error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint8_t cmd_sm9_exckey_gen_user_prikey(uint8_t *id, uint32_t id_len, uint8_t *ke, uint8_t *de)
{
    uint8_t ret = 0;
    uint32_t cmd_buf[8];
    cmd_sm9_exckey_generate_user_prikey_t *cmd_gen_user_key = NULL;

    /*********** generate user private key ***********/

    cmd_gen_user_key = (cmd_sm9_exckey_generate_user_prikey_t *)cmd_buf;
    cmd_gen_user_key->cmd_id = SM9_EXCKEY_GENERATE_USER_PRIVATE_KEY;

    set_big_endian_4byte((uint8_t *)cmd_gen_user_key->id_ptr, (uint32_t)id);
    set_big_endian_2byte((uint8_t *)cmd_gen_user_key->id_len, id_len);

    cmd_gen_user_key->hid = 0x2;
    set_big_endian_4byte((uint8_t *)cmd_gen_user_key->mastPriKey_ptr, (uint32_t)ke);
    set_big_endian_4byte((uint8_t *)cmd_gen_user_key->userPriKey_ptr, (uint32_t)de);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("gen user A pri key success id_len=%d \n", id_len);
    }
    else {
        print_buf_U8(id, id_len, "id");
        print_buf_U8(ke, 32, "ke");
        print_buf_U8(de, 128, "de");
        printf("SM9_EXCKEY_GENERATE_USER_PRIVATE_KEY A error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint8_t cmd_sm9_exckey_gen_user_tmpkey(uint8_t *id, uint32_t id_len, uint8_t *pub_e, uint8_t *r, uint8_t *R)
{
    uint8_t ret = 0;
    uint32_t cmd_buf[8];
    cmd_sm9_exckey_generate_user_tmpkey_t *cmd_gen_user_tmp_key = NULL;

    /*********** generate user tmp key pair ***********/

    cmd_gen_user_tmp_key = (cmd_sm9_exckey_generate_user_tmpkey_t *)cmd_buf;
    cmd_gen_user_tmp_key->cmd_id = SM9_EXCKEY_GENERATE_USER_TMP_KEY;

    set_big_endian_4byte((uint8_t *)cmd_gen_user_tmp_key->peer_id_ptr, (uint32_t)id);
    set_big_endian_2byte((uint8_t *)cmd_gen_user_tmp_key->peer_id_len, id_len);

    cmd_gen_user_tmp_key->hid = 0x2;
    set_big_endian_4byte((uint8_t *)cmd_gen_user_tmp_key->mastPubKey_ptr, (uint32_t)pub_e);
    set_big_endian_4byte((uint8_t *)cmd_gen_user_tmp_key->userTmpPriKey_ptr, (uint32_t)r);
    set_big_endian_4byte((uint8_t *)cmd_gen_user_tmp_key->userTmpPubKey_ptr, (uint32_t)R);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("gen user A tmp key pair success idb_len=%d \n", id_len);
    }
    else {
        print_buf_U8(id, id_len, "id");
        print_buf_U8(pub_e, 32, "pub_e");
        print_buf_U8(r, 32, "r");
        print_buf_U8(R, 64, "R");
        printf("SM9_EXCKEY_GENERATE_USER_TMP_KEY A error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}

uint32_t cmd_sm9_set_exckey_input_ptr(uint8_t *pub_e, uint8_t *dA, uint8_t *RA, uint8_t *rA, uint8_t *RB, uint8_t *ida, uint8_t *idb, cmd_sm9_exc_key_input_ptr_t *exckey_input)
{
    set_big_endian_4byte((uint8_t *)exckey_input->Ppub_e_ptr, (uint32_t)pub_e);
    set_big_endian_4byte((uint8_t *)exckey_input->deA_ptr, (uint32_t)dA);
    set_big_endian_4byte((uint8_t *)exckey_input->RA_ptr, (uint32_t)RA);
    set_big_endian_4byte((uint8_t *)exckey_input->rA_ptr, (uint32_t)rA);
    set_big_endian_4byte((uint8_t *)exckey_input->RB_ptr, (uint32_t)RB);
    set_big_endian_4byte((uint8_t *)exckey_input->IDA_ptr, (uint32_t)ida);
    set_big_endian_4byte((uint8_t *)exckey_input->IDB_ptr, (uint32_t)idb);
    return CMD_RETURN_SUCESS;
}

uint8_t cmd_sm9_exchange_key(cmd_sm9_exchangekey_role_e role, uint32_t ida_len, uint32_t idb_len,
                             uint32_t kByteLen, cmd_sm9_exc_key_input_ptr_t *exckey_input,
                             uint8_t *KA, uint8_t *S1, uint8_t *SA)
{
    uint8_t ret = 0;
    uint32_t cmd_buf[8];
    cmd_sm9_exchangekey_t *cmd_exchange_key = NULL;

    /*********** A side exchange key ***********/

    cmd_exchange_key = (cmd_sm9_exchangekey_t *)cmd_buf;
    cmd_exchange_key->cmd_id = SM9_EXCHANGE_KEY;

    cmd_exchange_key->role = role;
    set_big_endian_4byte((uint8_t *)cmd_exchange_key->input_ptr, (uint32_t)exckey_input);
    set_big_endian_2byte((uint8_t *)cmd_exchange_key->ida_len, ida_len);
    set_big_endian_2byte((uint8_t *)cmd_exchange_key->idb_len, idb_len);
    cmd_exchange_key->key_len = kByteLen;

    set_big_endian_4byte((uint8_t *)cmd_exchange_key->outputKey_ptr, (uint32_t)KA);
    set_big_endian_4byte((uint8_t *)cmd_exchange_key->s1_s2_ptr, (uint32_t)S1);
    set_big_endian_4byte((uint8_t *)cmd_exchange_key->sa_sb_ptr, (uint32_t)SA);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {
        printf("exchange key success A ida_len=%d, idb_len=%d, key_len=%d \n", ida_len, idb_len, kByteLen);
    }
    else {
        print_buf_U8(KA, kByteLen, "KA");
        print_buf_U8(S1, 32, "S1");
        print_buf_U8(SA, 32, "SA");
        printf("SM9_EXCHANGE_KEY A error, ret = 0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}