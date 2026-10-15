/*
* Copyright (c) 2019 Semidrive Semiconductor Inc.
* All rights reserved.
*
* Description: domain communication high level api for secure
*
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <debug.h>

#include <rpmsg.h>
#include <rpmsg_service.h>
#include <rpmsg_service_cfg.h>
#include "rpmsg_common.h"

#include "rpmsg_rpc.h"
#include "dcf_rpc.h"

/** mbox include six domain */
#define MAX_DCF_SERVICES 6

static rpmsg_rpc_service_t s_rpmsgRpc_ins[MAX_DCF_SERVICES];

int dcf_setup_func(rpmsg_rpc_service_t *hserver, rpc_server_impl_t *tables, int num)
{
    if (!hserver)
        return -1;

    if (!hserver->ready)
        return ERR_NOT_READY;

    return ipcc_rpc_setup_implement(&hserver->rpc_server, tables, num);
}

int dcf_setup_func_detail(rpmsg_rpc_service_t *hserver, uint32_t cmd, rpc_server_func_t fn)
{
    rpc_server_impl_t fd;

    if (!hserver)
        return -1;

    if (!hserver->ready)
        return -1;

    fd.command = cmd;
    fd.flags = IPCC_RPC_NO_FLAGS;
    fd.func = fn;

    return ipcc_rpc_setup_implement(&hserver->rpc_server, &fd, 1);
}

int __dcf_call(rpmsg_rpc_service_t *hserver, rpc_call_request_t *request, rpc_call_result_t *result, uint32_t timeout)
{
    if (!hserver)
        return -1;

    return ipcc_rpc_client_call(&hserver->rpc_client, request, result, timeout);
}

status_t __dcf_ping(rpmsg_rpc_service_t *hserver)
{
    rpc_call_result_t result = DCF_RPC_RES_INITVALUE;
    rpc_call_request_t request = DCF_RPC_REQ_INITVALUE;
    status_t ret = 0;

    DCF_INIT_RPC_REQ4(request, IPCC_RPC_REQ_PING, 0xff, 0xff00, 0xff0000, 0xff000000);
    ret = ipcc_rpc_client_call(&hserver->rpc_client, &request, &result, 1000);
    if (ret < 0) {
        ssdk_printf(0, "rpc: call_func:%x fail ret: %d\n", request.cmd, ret);
        return ret;
    }

    ret = result.retcode;
    ssdk_printf(1, "rpc: get result: %x %d %x %x %x\n", result.ack, ret,
                result.result[0], result.result[1], result.result[2]);
    assert(result.ack == IPCC_RPC_ACK_PING);
    assert(ret == NO_ERROR);

    if (memcmp((char*)request.param, (char*)result.result, sizeof(result.result)))
        ssdk_printf(0, "rpc: echo not match\n");

    return ret;
}

status_t __dcf_gettimeofday(rpmsg_rpc_service_t *hserver)
{
    rpc_call_result_t result = DCF_RPC_RES_INITVALUE;
    rpc_call_request_t request = DCF_RPC_REQ_INITVALUE;
    status_t ret = 0;

    DCF_INIT_RPC_REQ(request, IPCC_RPC_REQ_GETTIMEOFDAY);
    ret = ipcc_rpc_client_call(&hserver->rpc_client, &request, &result, 1000);
    if (ret < 0) {
        ssdk_printf(0, "rpc: call-func:%x fail ret: %d\n", request.cmd, ret);
        return ret;
    }
    ret = result.retcode;
    ssdk_printf(0, "rpc: get result: %x %d %x %x %x\n", result.ack, ret,
                result.result[0], result.result[1], result.result[2]);
    assert(result.ack == IPCC_RPC_ACK_GETTIMEOFDAY);
    assert(ret == NO_ERROR);
    return ret;
}

int32_t dcf_rpc_service_init(rpmsg_service_t* service)
{
    if (!service)
        return -1;

    if (service->rpmsg_type == RPMSG_TYPE_IPCC &&
        service->remote_proc < MAX_DCF_SERVICES) {
        int32_t rproc = service->remote_proc;
        struct rpmsg_instance* rpmsg_ins = rpmsg_get_instance(RPMSG_TYPE_IPCC, rproc);
        ipcc_rpc_client_init(&s_rpmsgRpc_ins[rproc].rpc_client, rpmsg_ins, IPCC_RPC_CLIENT_EPT, IPCC_RPC_MAIN_EPT);
        ipcc_rpc_server_init(&s_rpmsgRpc_ins[rproc].rpc_server, rpmsg_ins, IPCC_RPC_MAIN_EPT, 0);
        ipcc_rpc_server_start(&s_rpmsgRpc_ins[rproc].rpc_server);
        s_rpmsgRpc_ins[rproc].ready = true;
    }
    return 0;
}

int32_t dcf_rpc_service_deinit()
{
    for (int32_t cnt = 0; cnt < MAX_DCF_SERVICES; cnt++) {
        if (s_rpmsgRpc_ins[cnt].ready) {
            ipcc_rpc_client_destroy(&s_rpmsgRpc_ins[cnt].rpc_client);
            ipcc_rpc_server_stop(&s_rpmsgRpc_ins[cnt].rpc_server);
            ipcc_rpc_server_exit(&s_rpmsgRpc_ins[cnt].rpc_server);
            s_rpmsgRpc_ins[cnt].ready = false;
        }
    }

    return 0;
}

void start_ipcc_rpc_service(rpc_server_impl_t *tables, int num)
{
    for (int i = 0; i < RPMSG_SERVICES_NUM; i++) {
        if (g_rpmsg_services[i].rpmsg_type == RPMSG_TYPE_IPCC) {
            for (int j = 0; j < 100; j++) {
                if (!dcf_setup_func(&s_rpmsgRpc_ins[g_rpmsg_services[i].remote_proc], tables, num))
                    break;
                osDelay(50);
            }
        }
    }
}

void dcf_register_rpc_implement(rpc_server_impl_t *fn)
{
    unsigned int i;

    for (i = 0; i < MAX_DCF_SERVICES; i++) {
        dcf_setup_func(&s_rpmsgRpc_ins[i], fn, 1);
    }
}

void dcf_register_rpc_implement_detailed(uint32_t cmd, rpc_server_func_t fn)
{
    unsigned int i;

    for (i = 0; i < MAX_DCF_SERVICES; i++) {
        dcf_setup_func_detail(&s_rpmsgRpc_ins[i], cmd, fn);
    }
}

rpmsg_rpc_service_t* dcf_get_service_handle(int32_t rproc)
{
    if (rproc > MAX_DCF_SERVICES) {
        ssdk_printf(SSDK_EMERG, "rproc %d exceed limit service\n", rproc);
        return NULL;
    }

    return &s_rpmsgRpc_ins[rproc];
}
