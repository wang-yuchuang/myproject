/*
 * sdpe_rpc_service.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef SDPE_RPC_SERVICE_H
#define SDPE_RPC_SERVICE_H

#include <types.h>

enum sdpe_rpc_service_e
{
    SDPE_RPC_SDPE_SERVICE = 0,
    SDPE_RPC_VCAN_SERVICE,
    SDPE_RPC_VLIN_SERVICE,
    SDPE_RPC_VETH1_SERVICE,
    SDPE_RPC_VETH2_SERVICE,
    SDPE_RPC_VDOIP_SERVICE,
    SDPE_RPC_MAX_SERVICE
};

typedef struct sdpe_rpc_service_cfg {
    uint8_t         service;
    uint8_t         rproc;
    uint32_t        addr;
    void            *lower_half;
} sdpe_rpc_service_cfg_t;

typedef int (*sdpe_rpc_service_hdl)(void *arg, uint8_t *data, uint32_t len);

/**
 * @brief sdpe rpc service init.
 *
 * @param[in] cfg_list config list.
 */
void sdpe_rpc_service_init(void);

#endif
