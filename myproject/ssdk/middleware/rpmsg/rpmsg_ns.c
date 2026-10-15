/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * Copyright (c) 2015 Xilinx, Inc.
 * Copyright (c) 2016 Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "rpmsg.h"
#include "rpmsg_ns.h"
#include <types.h>
#include <string.h>
#include <sd_errcode.h>
#include "debug.h"
#include "compiler.h"

#define RPMSG_NS_NAME_SIZE (32)
#define RPMSG_NS_TIMEOUT   (3000)   /**< announcement timeout value */

/*!
 * struct rpmsg_ns_msg - dynamic name service announcement message
 * @name: name of remote service that is published
 * @addr: address of remote service that is published
 * @flags: indicates whether service is created or destroyed
 *
 * This message is sent across to publish a new service, or announce
 * about its removal. When we receive these messages, an appropriate
 * rpmsg channel (i.e device) is created/destroyed. In turn, the ->probe()
 * or ->remove() handler of the appropriate rpmsg driver will be invoked
 * (if/as-soon-as one is registered).
 */
struct rpmsg_ns_msg
{
    char name[RPMSG_NS_NAME_SIZE];
    uint32_t addr;
    uint32_t flags;
} __PACKED;

/*!
 * @brief
 * Nameservice callback, called in interrupt context
 *
 * @param payload     Pointer to the buffer containing received data
 * @param payload_len Size of data received, in bytes
 * @param src         Pointer to address of the endpoint from which data is received
 * @param priv        Private data provided during endpoint creation
 *
 * @return  RL_RELEASE, message is always freed
 *
 */
static int rpmsg_ns_rx_cb(void *payload, int payload_len, unsigned long src, void *priv)
{
    struct rpmsg_ns_msg *ns_msg_ptr = payload;
    struct rpmsg_ns_callback_data *cb_ctxt = priv;

    ASSERT(priv);
    ASSERT(cb_ctxt->cb);

    /* Drop likely bad messages received at nameservice address */
    if (payload_len == sizeof(struct rpmsg_ns_msg))
    {
        cb_ctxt->cb(ns_msg_ptr->addr, ns_msg_ptr->name, ns_msg_ptr->flags, cb_ctxt->user_data);
    }

    return RPMSG_RELEASE;
}

rpmsg_ns_handle rpmsg_ns_bind(struct rpmsg_instance *rpmsg_dev, rpmsg_ns_new_ept_cb app_cb, void *user_data)
{
    struct rpmsg_ns_context *ns_ctxt;

    if (app_cb == NULL)
    {
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_COMM,
                rpmsg_dev->remote_proc, COM_IPCC_INVALID_ARGS, 0);
        return NULL;
    }

    struct rpmsg_ns_callback_data *cb_ctxt;

    cb_ctxt = osAlloc(sizeof(struct rpmsg_ns_callback_data));
    if (cb_ctxt == NULL)
    {
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_COMM,
                rpmsg_dev->remote_proc, COM_IPCC_MALLOC_FAIL, 0);
        return NULL;
    }
    ns_ctxt = osAlloc(sizeof(struct rpmsg_ns_context));
    if (ns_ctxt == NULL)
    {
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_COMM,
                rpmsg_dev->remote_proc, COM_IPCC_MALLOC_FAIL, 0);
        osFree(cb_ctxt);
        return NULL;
    }

    /* Set-up the nameservice callback context */
    cb_ctxt->user_data = user_data;
    cb_ctxt->cb = app_cb;

    ns_ctxt->cb_ctxt = cb_ctxt;

    ns_ctxt->ept = rpmsg_create_ept(rpmsg_dev, RPMSG_NS_EPT_ADDR, rpmsg_ns_rx_cb, (void *)ns_ctxt->cb_ctxt);

    return (rpmsg_ns_handle)ns_ctxt;
}

int rpmsg_ns_unbind(struct rpmsg_instance *rpmsg_dev, rpmsg_ns_handle handle)
{
    int retval;
    struct rpmsg_ns_context *ns_ctxt = (struct rpmsg_ns_context *)handle;

    retval = rpmsg_destroy_ept(rpmsg_dev, ns_ctxt->ept);
    osFree(ns_ctxt->cb_ctxt);
    osFree(ns_ctxt);
    return retval;
}

int rpmsg_ns_announce(struct rpmsg_instance *rpmsg_dev,
                      struct rpmsg_endpoint *new_ept,
                      char *ept_name,
                      unsigned long flags)
{
    struct rpmsg_ns_msg ns_msg;

    if (ept_name == NULL)
    {
        return RPMSG_ERR_PARAM;
    }

    if (new_ept == NULL)
    {
        return RPMSG_ERR_PARAM;
    }

    strncpy(ns_msg.name, ept_name, RPMSG_NS_NAME_SIZE);
    ns_msg.flags = flags;
    ns_msg.addr = new_ept->addr;

    //@NOTE ns message setup timeout, avoid mbox message buffer block
    return rpmsg_send(rpmsg_dev, new_ept, RPMSG_NS_EPT_ADDR,
        (char *)&ns_msg, sizeof(struct rpmsg_ns_msg), RPMSG_NS_TIMEOUT);
}
