/*
 * sdpe_ctrl_service.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef SDPE_RPC_CTRL_SERVICE_H
#define SDPE_RPC_CTRL_SERVICE_H

#include <sdpe_rpc_service.h>

/* SDPE CTRL SERVICE */

/*
 * @brief Initializes sdpe rpc ctrl service.
 * @param[in] cfg service config
 * @return init result
 */
int sdpe_rpc_ctrl_service_init(sdpe_rpc_service_cfg_t *cfg);

/*
 * @brief Check sdpe rpc ctrl is ready.
 */
bool sdpe_rpc_is_ready(void);

/*
 * @brief sdpe rpc sync.
 */
void sdpe_rpc_sync(void);

/*
 * @brief Registe sdpe rpc ctrl service.
 * @param[in] service service id.
 * @param[in] usr_cb service callback
 * @return service device
 */
#ifndef CONFIG_SDPE_RPC_MULTI_TASK
void *sdpe_rpc_service_cb_register(uint8_t service, void *usr_cb);
#endif

/*
 * @brief sdpe rpc start routing.
 * @direction client->server
 * @param[in] route_table route table address.
 * @param[in] size route table size.
 */
void sdpe_rpc_start_routing(uint32_t route_table, uint32_t size);

/*
 * @brief sdpe rpc stop routing.
 * @direction client->server
 */
void sdpe_rpc_stop_routing(void);

/**
 * @brief sdpe rpc enable monitor event
 * @direction client->server
 * @param[in] event_id event id
 * @param[in] enable enable or disable event
 * @param[in] arg_len event arg length
 * @param[in] arg event arg
 */
void sdpe_rpc_monitor_event(uint32_t event_id, uint8_t enable,
                            uint32_t arg_len, uint8_t *arg);

/**
 * @brief sdpe rpc event callback
 * @direction server->client
 * @param[in] event_id event id
 * @param[in] arg event arg
 */
void sdpe_rpc_event_cb(uint32_t event_id, uint32_t arg);

#endif
