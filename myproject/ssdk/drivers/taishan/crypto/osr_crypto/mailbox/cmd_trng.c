/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <driver/taishan/crypto/osr_crypto/mailbox/cmd_trng.h>

uint32_t cmd_trng_get_rand(uint8_t *rand_output, uint32_t rand_bytes)
{
    uint32_t cmd_buf[8] = {0};
    uint32_t ret;
    uint32_t *rand;
    cmd_tng_t *cmd_get_random = NULL;

    rand = (uint32_t *)rand_output;
    cmd_get_random = (cmd_tng_t *)cmd_buf;
    cmd_get_random->cmd_id = GENERATE_RAND;

    set_big_endian_4byte((uint8_t *)cmd_get_random->random_ptr, (uint32_t)rand);
    set_big_endian_4byte((uint8_t *)cmd_get_random->random_len, (uint32_t)rand_bytes);

    ret = send_cmd_and_wait_response((uint32_t *)cmd_buf);

    if (SEIP_SUCCESS == ret) {

    }
    else {
        printf("get random failure, ret=0x%x \n", ret);
        return CMD_RETURN_FAIL;
    }

    return CMD_RETURN_SUCESS;
}