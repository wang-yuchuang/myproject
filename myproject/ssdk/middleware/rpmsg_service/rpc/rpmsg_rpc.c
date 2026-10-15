/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#include <reg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <rpmsg.h>
#include <rpmsg_channel.h>
#include <board.h>
#include <debug.h>

#include "rpmsg_rpc.h"

#define IPCC_RPCS_CHN_NAME	"rpmsg-ipcc-rpc"

#define IPC_RPC_DEFAULT_FUNCS     (16)

/* RPC server status */
#define IPCC_RPC_S_RUNNING        (1)
#define IPCC_RPC_S_STOPPED        (0)


int ipcc_rpc_client_init(rpc_client_handle_t hclient,
                       struct rpmsg_instance *dev, uint32_t client_id, uint32_t server_id)
{
    rpmsg_channel_t ichan;

    ichan = rpmsg_channel_create(dev, "rpc-client", client_id);
    if (!ichan) {
        free(hclient);
        ssdk_printf(SSDK_EMERG, "rpc: No channel available for client\n");
        return ERR_NO_RESOURCES;
    }

    rpmsg_channel_start(ichan, NULL, NULL);

    hclient->rpchan = ichan;
    hclient->service_id = server_id;
    ssdk_printf(SSDK_NOTICE, "rpc: channel %p created with clientId: 0x%x for %p\n", ichan, client_id, dev);
    return 0;
}

void ipcc_rpc_client_destroy(rpc_client_handle_t hclient)
{
    if (!hclient) {
        return;
    }

    if (hclient->rpchan) {
        rpmsg_channel_destroy(hclient->rpchan);
        hclient->rpchan = NULL;
        hclient->service_id = 0;
    }

}

status_t ipcc_rpc_client_call(rpc_client_handle_t hclient
                              , rpc_call_request_t *req
                              , rpc_call_result_t *result
                              , uint32_t timeout)
{
    status_t ret;
    int ret_size = sizeof(rpc_call_result_t);
    // unsigned long src;

    if (!hclient || !hclient->rpchan)
        return ERR_NOT_READY;

    ssdk_printf(SSDK_INFO, "rpc: channel %p called with req cmdId: 0x%x\n", hclient->rpchan, req->cmd);

    ret = rpmsg_channel_send(hclient->rpchan, hclient->service_id
                             , (char *)req, sizeof(rpc_call_request_t)
                             , timeout);
    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "rpc: ipcc send channel failed: ret: %d\n", ret);
        return ret;
    }
    ssdk_printf(SSDK_NOTICE, "rpc: cmd sent, wait response from srv=%d\n", hclient->service_id);

    /* TODO: use timeout reciever */
    ret = rpmsg_channel_recv(hclient->rpchan, (char *)result, sizeof(rpc_call_result_t), &ret_size, timeout);
    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "rpc: rx failed ret: %d\n", ret);
        return ret;
    }

    ssdk_printf(SSDK_INFO, "rpc: rx OK from %d\n", hclient->service_id);

    return ret;
}

rpc_call_result_t ipcc_rpc_server_ping(rpc_server_handle_t handle, rpc_call_request_t *req)
{
    rpc_call_result_t result;

    result.ack = IPCC_RPC_ACK_PING;
    result.retcode = NO_ERROR;
    result.result[0] = req->param[0];
    result.result[1] = req->param[1];
    result.result[2] = req->param[2];
    result.result[3] = req->param[3];

    return result;
}

rpc_call_result_t ipcc_rpc_server_gettimeofday(rpc_server_handle_t handle, rpc_call_request_t *req)
{
    rpc_call_result_t result;

    result.ack = IPCC_RPC_ACK_GETTIMEOFDAY;
    result.retcode = NO_ERROR;
    result.result[0] = current_time();
    result.result[1] = 0x00008800;
    result.result[2] = 0x00880000;
    result.result[3] = 0x88000000;

    return result;
}

/* Here defines builtin RPC functions shared by all RPC service */
static rpc_server_impl_t s_rpc_default_funcs[IPCC_RPC_FUNC_MAX] = {
    {IPCC_RPC_REQ_PING, ipcc_rpc_server_ping, IPCC_RPC_NO_FLAGS},
    {IPCC_RPC_REQ_GETTIMEOFDAY, ipcc_rpc_server_gettimeofday, IPCC_RPC_NO_FLAGS},
    {0, },
};

/* Try to find a RPC implement functions registered in the RPC service */
rpc_server_func_t find_func_implement(rpc_server_handle_t handle, uint32_t command)
{
    unsigned int i;
    rpc_server_impl_t *caller = &s_rpc_default_funcs[0];

    /* firstly find builtin functions */
    for (i = 0; i < ARRAY_SIZE(s_rpc_default_funcs); i++,  caller++) {
        if (caller->command == command) {
            return caller->func;
        }
    }

    /* then find user registered functions */
    if(handle->func_table) {
        caller = handle->func_table;
        for (i = 0; i < handle->func_used; i++, caller++) {
            if (caller->command == command) {
                return caller->func;
            }
        }
    }

    return NULL;
}

rpc_call_result_t ipcc_rpc_server_nofunc(rpc_server_handle_t handle, rpc_call_request_t *req)
{
    rpc_call_result_t result;

    result.ack = req->cmd;
    result.retcode = ERR_NOT_IMPLEMENTED;

    return result;
}

/* Call RPC implement function registered in the RPC service */
static void ipcc_rpc_server_call_reply(rpc_server_handle_t handle, rpc_call_request_t *req, int src)
{
    rpc_server_func_t func;
    rpc_call_result_t reply;
    status_t ret;

    func = find_func_implement(handle, req->cmd);

    if (func) {
        /* Execute this implement function */
        reply = func(handle, req);
        ssdk_printf(SSDK_DEBUG, "rpc: called from client: %d \n", src);

    } else {
        //@TODO can not get rproc info from rpmsg device instance
        // ssdk_printf(0, "rpc: Not found, cmd=0x%x from %d:%d\n",
        //            req->cmd, handle->rpdev, src);
        reply = ipcc_rpc_server_nofunc(handle, req);
    }

    /* if execution is no error, reply to the client in client_id */
    ret = rpmsg_channel_send(handle->rpchan
                             , src
                             , (char *)&reply
                             , sizeof(rpc_call_result_t)
                             , RPMSG_BLOCK);
    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "rpc: reply failed: ret: %d\n", ret);
        return;
    }

    ssdk_printf(SSDK_INFO, "rpc: reply cmd: 0x%x to %d done\n", req->cmd, src);
}

static int32_t rpc_server_dispatch(void* dcfMsg, int len, unsigned long src, void* usrCtx)
{
    rpc_server_handle_t handle = (rpc_server_handle_t)usrCtx;
    rpc_call_request_t request;
    struct dcf_message *mssg = (struct dcf_message*)dcfMsg;

    assert(handle);
    assert(mssg);
    assert(len == sizeof(rpc_call_request_t));
    memcpy(&request, mssg, len);

    ssdk_printf(SSDK_INFO, "rpc: Receive cmd: 0x%x from %ld\n", request.cmd, src);

    if (handle->status == IPCC_RPC_S_RUNNING) {
        ipcc_rpc_server_call_reply(handle, &request, src);
    }

    return 0;
}

int ipcc_rpc_server_init(rpc_server_handle_t handle,
                        struct rpmsg_instance *dev, int server_id, int threads)
{
    if (!dev) {
        ssdk_printf(SSDK_EMERG, "Bad ipcc device handle\n");
        return ERR_INVALID_ARGS;
    }

    handle->rpdev = dev;
    handle->status = IPCC_RPC_S_STOPPED;
    handle->rpchan = rpmsg_channel_create(dev, IPCC_RPCS_CHN_NAME, server_id);
    if (!handle->rpchan) {
        ssdk_printf(SSDK_EMERG, "rpc: No channel available\n");
        return ERR_NO_RESOURCES;
    }

    if (threads) {
        ssdk_printf(SSDK_ERR, "rpc: Thread mode is not supported yet!\n");
        return ERR_NOT_IMPLEMENTED;
    }

    handle->func_max = IPC_RPC_DEFAULT_FUNCS;
    handle->func_used = 0;
    handle->func_table = malloc(IPC_RPC_DEFAULT_FUNCS * sizeof(rpc_server_impl_t));
    if (!handle->func_table) {
        ssdk_printf(SSDK_EMERG, "rpc: failed to alloc function table (svc-%d) \n", server_id);
        return ERR_NO_MEMORY;
    }

    handle->service_id = server_id;
    rpmsg_channel_start(handle->rpchan, rpc_server_dispatch, handle);

    return NO_ERROR;
}

status_t ipcc_rpc_server_start(rpc_server_handle_t handle)
{
    handle->status = IPCC_RPC_S_RUNNING;

    return NO_ERROR;
}

status_t ipcc_rpc_server_stop(rpc_server_handle_t handle)
{
    handle->status = IPCC_RPC_S_STOPPED;

    return NO_ERROR;
}

status_t ipcc_rpc_setup_implement(rpc_server_handle_t handle,
                                         rpc_server_impl_t *tables, int num)
{
    rpc_server_impl_t *ptr = NULL;
    int left = handle->func_max - handle->func_used;

    if (left >= num) {
        ptr = handle->func_table + handle->func_used;
        handle->func_used += num;
    }

    if (!ptr) {
        ssdk_printf(SSDK_EMERG, "rpc: TODO: func table exceed max\n");
        ssdk_printf(SSDK_EMERG, "rpc: srv=%d left=%d func=%d\n",
                    handle->service_id,
                    left, handle->func_used);

        return ERR_NO_MEMORY;
    }

    memcpy(ptr, tables, sizeof(rpc_server_impl_t) * num);

    ssdk_printf(SSDK_NOTICE, "%d installed %d RPC functions \n",
                handle->service_id, handle->func_used);

    return NO_ERROR;
}

void ipcc_rpc_server_exit(rpc_server_handle_t handle)
{
    if (handle->func_table)
        free(handle->func_table);

    if (handle->rpchan)
        rpmsg_channel_destroy(handle->rpchan);

    return;
}
