/*
 * sdpe_ctrl_client.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdpe client driver provides remote function calls
 * to safety.
 *
 * Revision History:
 * -----------------
 */

#include <types.h>
#include <sdpe_rpc_ctrl_service.h>

#include "sdpe_ctrl_common.h"

/**
 * @brief start sdpe routing
 *
 * @param[in] route_table route table address
 * @param[in] size route table size
 */
void sdpe_start_routing(uint32_t route_table, uint32_t size)
{
    SDPE_CTRL_INFO("sdpe_starting_routing 0x%x\n", route_table);
    sdpe_rpc_start_routing(route_table, size);
}

/**
 * @brief stop sdpe routing
 */
void sdpe_stop_routing(void)
{
    SDPE_CTRL_INFO("sdpe_stop_routing\n");
    sdpe_rpc_stop_routing();
}

/**
 * @brief enable sdpe monitor event
 *
 * @param[in] event_id event id
 * @param[in] enable enable or disable event
 * @param[in] arg_len event arg length
 * @param[in] arg event arg
 */
void sdpe_monitor_event(uint32_t event_id, uint8_t enable, uint32_t arg_len, uint8_t *arg)
{
    SDPE_CTRL_INFO("%s: event %d, enable %d, arg %p, arg len %d\n", __func__,
                   event_id, enable, arg, arg_len);
    sdpe_rpc_monitor_event(event_id, enable, arg_len, arg);
}

/**
 * @brief sdpe event callback
 *
 * @param[in] event_id event id
 * @param[in] arg event arg
 */
void sdpe_rpc_event_cb(uint32_t event_id, uint32_t arg)
{
    SDPE_CTRL_ALWAYS("%s: %d %d\n", __func__, event_id, arg);
}