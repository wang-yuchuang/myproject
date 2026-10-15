/*
 * sdpe_rpc_pack.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include <string.h>

#include <param.h>
#include <cmsis_os2.h>

#include <sdpe_rpc_common.h>
#include <sdpe_rpc_device.h>

#include "sdpe_rpc_pack.h"

#define SF                              0xF0U                           /* Single frame */
#define MF(nr)                          ((nr) & 0x0FU)                  /* Multiple frame, nr: frame number */
#define EF(nr)                          (((nr) & 0x0FU) | 0x10U)        /* Ending frame */
#define IS_SF(buf)                      ((buf)[0] == SF)                /* Is single frame? */
#define IS_EF(buf)                      (((buf)[0] & 0xF0U) == 0x10U)   /* Is ending frame? */
#define GET_FRAME_NUM(buf)              ((buf)[0] & 0x0FU)
#define FC_SIZE                         1U

static void sdpe_rpc_pack_restart(sdpe_rpc_pack_info_t *pack)
{
    pack->pack_offset = 0;
    pack->pack_sn = 0;
}

static void sdpe_rpc_pack_fill_head(uint8_t *pdu, uint8_t frame_cnt, bool last)
{
    if (last) {
        if (frame_cnt == 0) {
            *pdu = SF;
        }
        else {
            *pdu = EF(frame_cnt);
        }
    }
    else {
        *pdu = MF(frame_cnt);
    }
}

int sdpe_rpc_dev_pack_init(sdpe_rpc_dev_t *dev)
{
    sdpe_rpc_pack_info_t *pack = &dev->pack;

    pack->pack_buf_len = sdpe_rpc_get_mtu(&dev->transport);

    pack->pack_buffer = osAlloc(pack->pack_buf_len);
    if (!pack->pack_buffer) {
        return -1;
    }

    return 0;
}

int sdpe_rpc_dev_pack_deinit(sdpe_rpc_dev_t *dev)
{
    sdpe_rpc_pack_info_t *pack = &dev->pack;

    if (pack->pack_buffer)
        osFree(pack->pack_buffer);

    return 0;
}

int sdpe_rpc_dev_pack_send(sdpe_rpc_dev_t *dev,
                           struct sdpe_rpc_message_head *rpc_header,
                           sdpe_rpc_message_t *message, uint8_t *buffer)
{
    uint8_t *sdu = NULL;
    uint8_t *pdu = buffer;
    uint32_t sdu_offset = 0;
    uint32_t pdu_offset = FC_SIZE;
    uint32_t sdu_len;
    uint32_t pdu_len;
    uint32_t copy_len;
    uint8_t frame_cnt = 0;
    uint8_t sdu_cnt;

    /* fill rpc header */
    memcpy(&pdu[pdu_offset], rpc_header, SDPE_RPC_MESSAGE_HEAD);
    pdu_offset += SDPE_RPC_MESSAGE_HEAD;

    /* fill rpc data */
    for (sdu_cnt = 0; sdu_cnt < message->data_num; sdu_cnt++) {
        sdu = message->data[sdu_cnt];
        sdu_len = message->data_len[sdu_cnt];
        sdu_offset = 0;

        while (sdu_offset < sdu_len) {

            if (!pdu) {
                pdu_len = SDPE_RPC_MESSAGE_HEAD;
                pdu = sdpe_rpc_transport_tx_alloc(&dev->transport, &pdu_len,
                                                  SDPE_RPC_WAIT_FOREVER);
                if (!pdu) {
                    SDPE_RPC_ERR("allocates tx buffer failed\n");
                    return -1;
                }
                pdu_offset = FC_SIZE;
            }

            /* copy sdu data to pdu */

            copy_len = MIN((sdu_len - sdu_offset), (pdu_len - pdu_offset));
            memcpy(&pdu[pdu_offset], &sdu[sdu_offset], copy_len);
            sdu_offset += copy_len;
            pdu_offset += copy_len;

            /* finish pdu or no data to trans, send pdu */

            bool last_end = (sdu_offset == sdu_len) && ((message->data_num - sdu_cnt) == 1);

            if (pdu_offset == pdu_len || last_end) {

                /* fill pdu head */
                sdpe_rpc_pack_fill_head(pdu, frame_cnt, last_end);
                frame_cnt++;

                sdpe_rpc_transport_send_nocopy(&dev->transport, pdu, pdu_offset);
                pdu = NULL;
            }
        }
    }

    if (pdu) {
        sdpe_rpc_pack_fill_head(pdu, frame_cnt, true);
        sdpe_rpc_transport_send_nocopy(&dev->transport, pdu, pdu_offset);
    }

    return 0;
}

int sdpe_rpc_dev_unpack_recv(sdpe_rpc_dev_t *dev, uint8_t *data, uint32_t *data_len)
{
    sdpe_rpc_pack_info_t *pack = &dev->pack;
    uint8_t sn;
    uint16_t payload;
    uint32_t recv_len;

    while (1) {

        recv_len = pack->pack_buf_len;

        if (sdpe_rpc_transport_recv(&dev->transport, pack->pack_buffer,
                                    &recv_len, true) < 0) {
            SDPE_RPC_ERR("rpc transport recv error...\n");
            return -1;
        }

        payload = recv_len - FC_SIZE;

        if ((payload + pack->pack_offset) > *data_len) {
            SDPE_RPC_ERR("recv frame len error...\n");
            sdpe_rpc_pack_restart(pack);
            return -1;
        }

        sn = GET_FRAME_NUM(pack->pack_buffer);

        if (IS_SF(pack->pack_buffer) || sn == 0) {
            sdpe_rpc_pack_restart(pack);
        }
        else if (sn == (pack->pack_sn + 1)) {
            pack->pack_sn = sn;
        }
        else {
            SDPE_RPC_ERR("recv frame discontinuity...\n");
            sdpe_rpc_pack_restart(pack);
            return -1;
        }

        memcpy(&data[pack->pack_offset], pack->pack_buffer + FC_SIZE, payload);
        pack->pack_offset += payload;

        if (IS_SF(pack->pack_buffer) || IS_EF(pack->pack_buffer)) {
            *data_len = pack->pack_offset;
            sdpe_rpc_pack_restart(pack);
            return 0;
        }
    }
}

