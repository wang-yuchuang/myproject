/*
 * sdpe_ctrl_server.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: SDPE Server driver handles remote function calls, by
 * safety.
 *
 * Revision History:
 * -----------------
 */

#include <sdpe/event.h>

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
    sdpe_start((void *)route_table, size);
}

/**
 * @brief stop sdpe routing
 */
void sdpe_stop_routing(void)
{
    sdpe_stop();
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

    if (event_id == ROUTE_STAT) {
        stat_filter_enable((void *)arg, enable);
    }
    else if (event_id == ECU_NODE_MISS) {
        node_loss_filter_enable((void *)arg, enable);
    }
    else if (event_id == PKT_ID_MISSING) {
        msg_timeout_filter_enable((void *)arg, enable);
    }
    else if (event_id == LIVE_COUNTER) {
        live_cnt_filter_enable((void *)arg, enable);
    }
    else if (event_id == CHECKSUM_ERR) {
        checksum_filter_enable((void *)arg, enable);
    }
    else if (event_id == PACKET_OVERFLOW) {
        pkt_ovf_filter_enable((void *)arg, enable);
    }
    else if (event_id == BUS_MIRROR) {
        mirror_filter_enable((void *)arg, enable);
    }
    else if (event_id == INVALID_PACKET) {
        invasion_filter_enable((void *)arg, enable);
    }
}

/**
 * @brief sdpe event callback
 *
 * @param[in] event_id event id
 * @param[in] arg event arg
 */
void sdpe_event_cb(uint32_t event_id, uint32_t arg)
{
    sdpe_rpc_event_cb(event_id, arg);
}
