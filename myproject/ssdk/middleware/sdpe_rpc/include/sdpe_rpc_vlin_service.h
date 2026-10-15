/*
 * vlin_service.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef SDPE_RPC_VLIN_SERVICE_H
#define SDPE_RPC_VLIN_SERVICE_H

#include <sdpe_rpc_service.h>
#include <vlin.h>

/* VLIN SERVICE */

/**
 * @brief sdpe rpc lin service init
 *
 * @param[in] cfg lin service config info
 * @return result
 */
int vlin_service_init(sdpe_rpc_service_cfg_t *cfg);

/**
 * @brief sdpe rpc send lin init message
 * @direction client->server
 * @param[in] config lin config info
 * @return result
 */
int virLin_Init(const Lin_ConfigType *config);

/**
 * @brief sdpe rpc send check wakeup message
 * @direction client->server
 * @param[in] channel channel id
 * @return result
 */
int virLin_CheckWakeup(uint8_t channel);

/**
 * @brief sdpe rpc send goto sleep message
 * @direction client->server
 * @param[in] channel channel id
 * @return result
 */
int virLin_GoToSleep(uint8_t channel);

/**
 * @brief sdpe rpc send goto sleep internal message
 * @direction client->server
 * @param[in] channel channel id
 * @return result
 */
int virLin_GoToSleepInternal(uint8_t channel);

/**
 * @brief sdpe rpc send wakeup message
 * @direction client->server
 * @param[in] channel channel id
 * @return result
 */
int virLin_Wakeup(uint8_t channel);

/**
 * @brief sdpe rpc send wakeup internal message
 * @direction client->server
 * @param[in] channel channel id
 * @return result
 */
int virLin_WakeupInternal(uint8_t channel);

/**
 * @brief sdpe rpc send get status message
 * @direction client->server
 * @param[in] channel channel id
 * @param[in] lin_sdu sdu status
 * @return result
 */
int virLin_GetStatus(uint8_t channel, uint8_t *lin_sdu);

/**
 * @brief sdpe rpc send frame message
 * @direction client->server
 * @param[in] channel channel id
 * @param[in] channel pdu info
 * @return result
 */
int virLin_SendFrame(uint8_t channel, const Lin_PduType *pdu);

#endif
