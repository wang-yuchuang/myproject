/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <driver/taishan/crypto/osr_crypto/mailbox/cmd_hash.h>

uint32_t cmd_hash_calc(uint8_t mode, uint8_t *msg, uint32_t msg_bytes, uint8_t *digest)
{
    uint32_t cmd_buf[8];
    uint32_t ret;
    cmd_hash_t *cmd_generate_digest = NULL;

    /*********** generate digest ***********/
    cmd_generate_digest = (cmd_hash_t *)cmd_buf;
    cmd_generate_digest->cmd_id = HASH_CALC;
    cmd_generate_digest->mode = mode;

    set_big_endian_4byte((uint8_t *)cmd_generate_digest->data_ptr, (uint32_t)msg);
    set_big_endian_4byte((uint8_t *)cmd_generate_digest->data_len, (uint32_t)msg_bytes);
    set_big_endian_4byte((uint8_t *)cmd_generate_digest->digest_ptr, (uint32_t)digest);
    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {

    }
    else {
        printf("HASH generate digest failure, mode=%d, ret=0x%x \n", mode, ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}
