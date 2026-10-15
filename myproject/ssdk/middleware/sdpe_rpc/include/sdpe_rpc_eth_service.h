/*
 * eth_service.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef ETH_SERVICE_H
#define ETH_SERVICE_H

/* ETH SERVICE */

#include <sdpe_rpc_service.h>

#define SDPE_RPC_MAX_ETH_NUM    2

typedef int (*veth_usr_cb)(void *arg, uint8_t *data, uint32_t len);

int veth_service_init(sdpe_rpc_service_cfg_t *cfg);
int veth_service_open(uint8_t eth_id, veth_usr_cb usr_cb, void *arg);
int veth_service_close(uint8_t eth_id);
int veth_send_frame(uint8_t eth_id, uint8_t *data, uint32_t len);

#endif
