/*
 * sdpe_ctrl.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef SDPE_CTRL_H
#define SDPE_CTRL_H

#include <types.h>

/**
 * @brief start sdpe routing
 *
 * @param[in] route_table route table address
 * @param[in] size route table size
 */
void sdpe_start_routing(uint32_t route_table, uint32_t size);

/**
 * @brief stop sdpe routing
 */
void sdpe_stop_routing(void);

/**
 * @brief enable sdpe monitor event
 *
 * @param[in] event_id event id
 * @param[in] enable enable or disable event
 * @param[in] arg_len event arg length
 * @param[in] arg event arg
 */
void sdpe_monitor_event(uint32_t event_id, uint8_t enable, uint32_t arg_len, uint8_t *arg);

/**
 * @brief sdpe event callback
 *
 * @param[in] event_id event id
 * @param[in] arg event arg
 */
void sdpe_event_cb(uint32_t event_id, uint32_t arg);

#endif
