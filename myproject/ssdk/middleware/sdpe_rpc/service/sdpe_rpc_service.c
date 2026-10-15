/*
 * sdpe_rpc_service.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <types.h>
#include <param.h>

#include <sdpe_rpc_service.h>
#include <sdpe_rpc_ctrl_service.h>
#include <sdpe_rpc_vcan_service.h>
#include <sdpe_rpc_vlin_service.h>
#include <sdpe_rpc_eth_service.h>
#include <sdpe_rpc_doip_service.h>
#include <sdpe_rpc_common.h>

#include <sdpe_rpc_cfg.h>

void sdpe_rpc_service_init(void)
{
    sdpe_rpc_service_cfg_t service_list[] = SDPE_RPC_SERVICE_TABLE;

    for (uint8_t cnt = 0; cnt < ARRAY_SIZE(service_list); cnt++) {
        sdpe_rpc_service_cfg_t *cfg = &service_list[cnt];
        switch (cfg->service) {
#ifdef CONFIG_SDPE_RPC_MBOX_TRANSPORT
            case SDPE_RPC_SDPE_SERVICE:
                sdpe_rpc_ctrl_service_init(cfg);
                break;
            case SDPE_RPC_VCAN_SERVICE:
                vcan_service_init(cfg);
                break;
            case SDPE_RPC_VLIN_SERVICE:
                vlin_service_init(cfg);
                break;
#endif
#ifdef CONFIG_SDPE_RPC_RPMSG_TRANSPORT
            case SDPE_RPC_VETH1_SERVICE:
            case SDPE_RPC_VETH2_SERVICE:
                veth_service_init(cfg);
                break;
            case SDPE_RPC_VDOIP_SERVICE:
                vdoip_service_init(cfg);
                break;
#endif
            default:
                SDPE_RPC_ERR("SDPE RPC Wrong Service:%d\n", cfg->service);
                break;
        }
    }

    sdpe_rpc_sync();
}
