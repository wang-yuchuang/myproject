/*
 * vcan_service.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef SDPE_RPC_VCAN_SERVICE_H
#define SDPE_RPC_VCAN_SERVICE_H

#include <sdpe_rpc_service.h>
#include <vcan.h>

/* VCAN SERVICE */

/*
 * @brief Initializes sdpe rpc vcan service.
 * @param[in] cfg service config
 * @return init result
 */
int vcan_service_init(sdpe_rpc_service_cfg_t *cfg);

/*
 * @brief sdpe rpc send can init message.
 * @direction client->server
 * @param[in] config can config.
 */
void virCan_Init(const Can_ConfigType *config);

/*
 * @brief sdpe rpc send can deinit message.
 * @direction client->server
 */
void virCan_DeInit(void);

/*
 * @brief sdpe rpc send can set baudrate message.
 * @direction client->server
 * @param[in] controller can controller id.
 * @param[in] baudrate controller baudrate.
 * @return result.
 */
int virCan_SetBaudrate(uint8_t controller, uint16_t baudrate);

/*
 * @brief sdpe rpc send can set mode message.
 * @direction client->server
 * @param[in] controller can controller id.
 * @param[in] mode controller mode.
 * @return result.
 */
int virCan_SetControllerMode(uint8_t controller, uint8_t mode);

/*
 * @brief sdpe rpc send can get mode message.
 * @direction client->server
 * @param[in] controller can controller id.
 * @param[out] mode controller mode.
 * @return result.
 */
int virCan_GetControllerMode(uint8_t controller, uint8_t *mode);

/*
 * @brief sdpe rpc send enable interrupt message.
 * @direction client->server
 * @param[in] controller can controller id.
 * @return result.
 */
int virCan_EnableControllerInterrupts(uint8_t controller);

/*
 * @brief sdpe rpc send disable interrupt message.
 * @direction client->server
 * @param[in] controller can controller id.
 * @return result.
 */
int virCan_DisableControllerInterrupts(uint8_t controller);

/*
 * @brief sdpe rpc send check wakeup message.
 * @direction client->server
 * @param[in] controller can controller id.
 * @return result.
 */
int virCan_CheckWakeup(uint8_t controller);

/*
 * @brief sdpe rpc send get error state message.
 * @direction client->server
 * @param[in] controller can controller id.
 * @param[out] state error state.
 * @return result.
 */
int virCan_GetControllerErrorState(uint8_t controller, uint8_t *state);

/*
 * @brief sdpe rpc send can write message.
 * @direction client->server
 * @param[in] hth hth id.
 * @param[int] pdu info.
 * @return result.
 */
int virCan_Write(uint16_t hth, const Can_PduType *pdu);

/*
 * @brief sdpe rpc send mainfunction read message.
 * @direction client->server
 */
void vircan_MainFunction_Read(void);

/*
 * @brief sdpe rpc send mainfunction write message.
 * @direction client->server
 */
void vircan_MainFunction_Write(void);

/*
 * @brief sdpe rpc send controller bus off message.
 * @direction server->client
 * @param[in] controller can controller id.
 */
void virCan_ControllerBusOff(uint8_t controller);

/*
 * @brief sdpe rpc send wakeup event message.
 * @direction server->client
 * @param[in] controller can controller id.
 */
void virCan_SetWakeupEvent(uint8_t controller);

/*
 * @brief sdpe rpc send rx indication message.
 * @direction server->client
 * @param[in] hrh hrh id.
 * @param[in] id pdu id.
 * @param[in] dlc pdu len.
 * @param[in] pdu pdu info.
 */
void virCan_RxIndication(uint16_t hrh, uint32_t id, uint8_t dlc,
                         uint8_t *pdu);

/*
 * @brief sdpe rpc send rx conf message.
 * @direction server->client
 * @param[in] pdu_id pdu id.
 */
void virCan_TxConfirmation(uint16_t pdu_id);

/*
 * @brief sdpe rpc send wakeup message.
 * @direction server->client
 * @param[in] controller controller id.
 */
void virCan_ControllerWakeUp(uint8_t controller);

#endif
