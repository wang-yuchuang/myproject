/**
 * @file rpmsg_echo.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#include <string.h>
#include <param.h>
#include <debug.h>
#include <cmsis_os2.h>
#include <rpmsg_ns.h>
#include <rpmsg_virtio.h>
#include <rpmsg_ipcc.h>
#include <rpmsg_channel.h>

#ifdef CONFIG_RPMSG_WITH_LK
#include <driver/timer.h>
#include <driver/kunlun/syscnt/sdrv_syscnt_drv.h>
#endif /* CONFIG_RPMSG_WITH_LK */

#include "rpmsg_service.h"
#include "rpmsg_echo.h"
#include "sysmgr.h"

/* BOOT_READY 握手计数 — Phase 1 事件驱动 */
static int boot_ready_count = 0;
#ifdef CONFIG_RPMSG_WITH_LK
extern sdrv_syscnt_dev_t g_sdrv_syscnt_dev;
extern timer_dev_t *g_syscnt_timer;

static int echo_channel_cb(void *data, int len, unsigned long src, void *arg)
{
    rpmsg_channel_t chan = arg;
    struct dcf_ccm_hdr *ccm_pkg = data;
    int ret = 0;

    if (ccm_pkg->dmsg.msg_type == COMM_MSG_CCM_ECHO) {
        ccm_pkg->time[2] = timer_get_current_time(g_syscnt_timer) * g_sdrv_syscnt_dev.cnt_per_us;
        ret = rpmsg_channel_send(chan, src, data, len, 1000);
    }
    else if (ccm_pkg->dmsg.msg_type == COMM_MSG_CCM_BOOT_READY) {
        /* Phase 1: 反射回 A 核 + 计数，达 3 次后通知 sysmgr */
        ret = rpmsg_channel_send(chan, src, data, len, 1000);
        if (boot_ready_count < 3) {
            boot_ready_count++;
            ssdk_printf(SSDK_INFO, "echo_cb: BOOT_READY %d/3\n", boot_ready_count);
            if (boot_ready_count >= 3) {
                printf("[echo_cb] BOOT_READY ×3 complete, signaling AP ready\n");
                sysmgr_signal_ap_ready();
            }
        }
    }
    else if (ccm_pkg->dmsg.msg_type == COMM_MSG_CCM_ACK) {
        ret = rpmsg_channel_send(chan, src, (char *)"ACK", 4, 1000);
    }
    else {
        ssdk_printf(SSDK_WARNING, "echo_channel_cb: unknown msg_type 0x%02x %d bytes from addr %ld\n",
                   ccm_pkg->dmsg.msg_type, len, src);
    }

    if (ret != 0) {
        ssdk_printf(SSDK_WARNING, "echo_channel_cb: channel send failed\n");
    }

    return ret;
}
#else
static int echo_channel_cb(void *data, int len, unsigned long src, void *arg)
{
    struct rpmsg_echo_hdr *echo_msg = data;
    rpmsg_channel_t chan = arg;
    int ret = 0;

    if (echo_msg->magic == COMM_MSG_CCM_ECHO) {
        ret = rpmsg_channel_send(chan, src, data, len, 1000);
    }
    else if (echo_msg->magic == COMM_MSG_CCM_BOOT_READY) {
        /* Phase 1: 反射回 A 核 + 计数（非 XIP 模式） */
        ret = rpmsg_channel_send(chan, src, data, len, 1000);
        if (boot_ready_count < 3) {
            boot_ready_count++;
            ssdk_printf(SSDK_INFO, "echo_cb: BOOT_READY %d/3\n", boot_ready_count);
            if (boot_ready_count >= 3) {
                printf("[echo_cb] BOOT_READY ×3 complete, signaling AP ready\n");
                sysmgr_signal_ap_ready();
            }
        }
    }
    else if (echo_msg->magic == COMM_MSG_CCM_ACK) {
        ret = rpmsg_channel_send(chan, src, (char *)"ACK", 4, 1000);
    }
    else {
        ssdk_printf(SSDK_INFO, "echo_cb: unknown magic 0x%08x %d bytes from addr %ld\n",
                   echo_msg->magic, len, src);
    }

    if (ret != 0) {
        ssdk_printf(SSDK_WARNING, "echo_channel_cb: channel send failed\n");
    }

    return ret;
}
#endif /* CONFIG_RPMSG_WITH_LK */

void rpmsg_service_echo_init(struct rpmsg_instance *rpmsg_dev, uint8_t dev_type)
{
    rpmsg_channel_t echo_channel;
    char *echo_ept_name[RPMSG_TYPE_NUM] = {
        "rpmsg-echo", "ipcc-echo"
    };

    ASSERT(dev_type < RPMSG_TYPE_NUM);

    echo_channel = rpmsg_channel_create(rpmsg_dev, echo_ept_name[dev_type], RPMSG_ECHO_EPT_ADDR);
    if (echo_channel == NULL) {
        ssdk_printf(SSDK_ERR, "rpmsg_service_echo_init create echo channel failed\n");
        return;
    }

    if (rpmsg_channel_start(echo_channel, echo_channel_cb, echo_channel) != 0) {
        ssdk_printf(SSDK_ERR, "rpmsg_service_echo_init start echo channel failed\n");
        return;
    }
}

/* Phase 1: 重置 BOOT_READY 接收计数（AP 核重启时调用） */
void echo_reset_boot_ready_count(void)
{
    boot_ready_count = 0;
    printf("[echo_cb] BOOT_READY count reset\n");
}

