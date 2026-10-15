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
#ifndef RPMSG_IPCC_H_
#define RPMSG_IPCC_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <types.h>
#include <cmsis_os2.h>
#include <driver/mbox.h>
#include <lib/buffqueue.h>

#if defined(CONFIG_KUNLUN_MBOX)
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#elif defined(CONFIG_TAISHAN_MBOX)
#include <driver/taishan/mbox/sdrv_taishan_mbox_drv.h>
#endif

struct rpmsg_ipcc_config
{
    uint32_t self_proc;
    uint32_t remote_proc;
    uint16_t src_addr;
    uint16_t dst_addr;
    struct mbox_dev *mbox_ins;
    bool is_master;
};

struct rpmsg_ipcc_instance
{
    struct mbox_chan *chan;
    uint32_t link_state;
    buffqueue_t recv_queue;
    osThreadId_t thread;
    osEventFlagsId_t event;
    bool is_master;
};

int rpmsg_ipcc_probe(struct rpmsg_instance *rpmsg,
                     struct rpmsg_ipcc_instance *ipcc,
                     struct rpmsg_ipcc_config *cfg);

#if defined(__cplusplus)
}
#endif

#endif /* RPMSG_IPCC_H_ */
