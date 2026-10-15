/*
 * sdpe_rpc_pack.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef SDPE_RPC_PACK_H
#define SDPE_RPC_PACK_H

/**
 * @brief pack init
 *
 * @param[in] dev sdpe rpc dev
 * @return result
 */
int sdpe_rpc_dev_pack_init(sdpe_rpc_dev_t *dev);

/**
 * @brief pack uninit
 *
 * @param[in] dev sdpe rpc dev
 * @return result
 */
int sdpe_rpc_dev_pack_deinit(sdpe_rpc_dev_t *dev);

/**
 * @brief pack message to send
 *
 * @param[in] dev sdpe rpc dev
 * @return result
 */
int sdpe_rpc_dev_pack_send(sdpe_rpc_dev_t *dev,
                           struct sdpe_rpc_message_head *rpc_header,
                           sdpe_rpc_message_t *message, uint8_t *buffer);

/**
 * @brief unpack receive message
 *
 * @param[in] dev sdpe rpc dev
 * @return result
 */
int sdpe_rpc_dev_unpack_recv(sdpe_rpc_dev_t *dev, uint8_t *data, uint32_t *data_len);

#endif
