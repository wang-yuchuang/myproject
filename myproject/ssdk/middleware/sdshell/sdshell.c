/*
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <arch.h>
#include <arch/register.h>
#include <rpmsg_channel.h>
#include <rpmsg_service.h>
#include <rpmsg_echo.h>
#include <stdbool.h>
#include <sdshell.h>
#include <CLI.h>

#define SDSHELL_MAX_RPMSG_SIZE  (512)

typedef enum _sdshell_ctrl_idx_em {
    sdshell_ctrl_idx_ap1 = 0,
    sdshell_ctrl_idx_mp,
    sdshell_ctrl_idx_max
} sdshell_ctrl_idx_em;

static char vuart_tx_buf[LINE_TXBUF_SIZE];

typedef struct sdshell_service_control {
    bool xcon_en;
    bool logbuf_en;
    bool thread_running;
    uint32_t rproc;
    uint32_t core_id;
    uint32_t ept_id;
    char* chan_name;
    uint32_t bk_len[SDRV_LOGBUG_SEGMENT_NUM];
    char *txbuf[SDRV_LOGBUG_SEGMENT_NUM];
    struct rpmsg_instance *dev;
    struct rpmsg_channe *chan;
    osThreadId_t *thread;
    osThreadId_t *recv_thread;
    timer_t timer;
    osEventFlagsId_t event;
    sdshell_msg_t msg;
} sdshell_ctl_t;

static sdshell_ctl_t sdshell_ctl[sdshell_ctrl_idx_max] = {
    [sdshell_ctrl_idx_ap1] = {
        .rproc = RPMSG_RRPOC_AP1,
        .ept_id = SDSHELL_SERVICE_EPT,
        .chan_name = SDSHELL_SERVICE_NAME,
    },
    [sdshell_ctrl_idx_mp]  = {
        .rproc = RPMSG_RRPOC_MP,
        .ept_id = SDSHELL_MP_EPT,
        .chan_name = SDSHELL_MP_NAME,
    },
};

static sdshell_ctl_t *sdshell_ctl_get(int index)
{
    if (index >= 0 && index < sdshell_ctrl_idx_max)
        return &sdshell_ctl[index];
    return NULL;
}

static int sdshell_send_msg(sdshell_ctl_t* ctl, char *buf, int len)
{
    if (!ctl || !buf)
        return -1;

    int slen = 0;
    int pos = 0;
    int ret;

    while (pos < len) {
        if (MAX_IPCC_BUFF_SIZE + pos < len)
            slen = MAX_IPCC_BUFF_SIZE;
        else
            slen = len - pos;

        ret = rpmsg_channel_send(ctl->chan, ctl->ept_id, (void *)(buf + pos),
                                  slen, 500);

        if (ret < 0)
            return ret;

        pos += slen;
    }

    return 0;
}

int sdshell_wake_thread(void)
{
    for (int i = 0; i < sdshell_ctrl_idx_max; i++) {
        sdshell_ctl_t *ctl = sdshell_ctl_get(i);
        int cpsr;

        if (ctl->xcon_en && (!ctl->thread_running) && ctl->logbuf_en) {
            cpsr = read_cpsr() & CPSR_MODE_MASK;
            if (cpsr == CPSR_MODE_ABT || cpsr == CPSR_MODE_UND)
                return 0;
            osEventFlagsSet(ctl->event, 1);
        }
    }

   return 0;
}

static int sdshell_resource_free(sdshell_ctl_t *ctl)
{
    ctl->core_id = SDRV_CORE_MAX;
    ctl->xcon_en = 0;
    return 0;
}

static int sdshell_event_handle(sdshell_ctl_t* ctl, void *data, int len, unsigned long src,
                                void *arg)
{
    if (!ctl || !data)
        return -1;

    int i;
    char *cmdbuf;
    bool newline = false;
    memcpy(&ctl->msg, (char *)data, sizeof(ctl->msg));

    if (ctl->msg.type >= SDSHELL_MSG_MAX) {
        ssdk_printf(SSDK_ALERT, "%s: invalid message type.\n", __func__);
        return -1;
    }

    if ((ctl->msg.type != SDSHELL_MSG_TARGET_SET)
            && (ctl->core_id >= SDRV_CORE_MAX))
        return -1;

    switch (ctl->msg.type) {
        case SDSHELL_MSG_TARGET_SET:
            if (ctl->msg.size > 0) {
                memcpy((void *)&ctl->core_id, (char *)data + sizeof(ctl->msg),
                       ctl->msg.size);

                if (ctl->core_id >= SDRV_CORE_MAX) {
                    ssdk_printf(SSDK_ALERT, "%s: invalid core type = %d\n", __func__, ctl->core_id);
                    return -1;
                }

                if (virt_uart_is_available())
                    ctl->logbuf_en = 1;
                else
                    ctl->logbuf_en = 0;
            }

            break;

        case SDSHELL_MSG_READ:
            if (!ctl->logbuf_en) {
                ssdk_printf(SSDK_ALERT, "%s: last log buffer is not available.\n", __func__);
                return -1;
            }

            if (virt_uart_get_last_buf_and_size(ctl->txbuf, ctl->bk_len, SDRV_LOGBUG_SEGMENT_NUM) < 0)
                return -1;

            for (i = 0; i < SDRV_LOGBUG_SEGMENT_NUM; i++) {
                if (ctl->bk_len[i] > 0) {
                    sdshell_send_msg(ctl, ctl->txbuf[i], ctl->bk_len[i]);
                    if (ctl->txbuf[i][ctl->bk_len[i] -1] != '\n')
                        newline = true;
                    else
                        newline = false;
                }
            }

            if (newline)
                sdshell_send_msg(ctl, (char *)"\n", 1);

            break;

        case SDSHELL_MSG_RUN_CMD:
            if (ctl->logbuf_en)
                virt_uart_update_tail();

            if (ctl->msg.size > 0) {
                ctl->xcon_en = 1;
                cmdbuf = (char *)data + sizeof(ctl->msg);
                ssdk_printf(SSDK_WARNING, "%s: cmdbuf = %s, size = %d\n", __func__, cmdbuf, ctl->msg.size);
                sdshell_save_char(cmdbuf, ctl->msg.size);
            }

            break;

        case SDSHELL_MSG_PRINTON:
            if (!ctl->logbuf_en)
                return -1;

            virt_uart_update_tail();
            ctl->xcon_en = 1;
            break;

        case SDSHELL_MSG_PRINTOFF:
            ctl->xcon_en = 0;
            break;

        case SDSHELL_MSG_CLEAR:
            if (!ctl->logbuf_en)
                return -1;

            if (!ctl->thread_running)
                virt_uart_clear_buf();

            break;

        case SDSHELL_MSG_QUIT:
            sdshell_resource_free(ctl);
            break;
    }

    ctl->msg.type = SDSHELL_MSG_MAX;
    ssdk_printf(SSDK_WARNING, "%s: ctl->msg.type = %d, ctl->msg.size = %d, len = %d\n", __func__,
            ctl->msg.type, ctl->msg.size, len);
    return 0;
}

static void sdshell_recv_task(void* args)
{
    sdshell_ctl_t* ctl = (sdshell_ctl_t*)args;
    struct rpmsg_channel* rpmsg_chan = (struct rpmsg_channel*)ctl->chan;

    if (!rpmsg_chan) {
        ssdk_printf(SSDK_ERR, "%s rpmsg channel is NULL\n", __func__);
        return;
    }

    char* msg_buf = malloc(SDSHELL_MAX_RPMSG_SIZE);
    if (!msg_buf) {
        ssdk_printf(SSDK_ERR, "%s malloc msg_buf fail\n", __func__);
        return;
    }

    while (1) {
        int msg_len = 0;
        memset(msg_buf, 0, SDSHELL_MAX_RPMSG_SIZE);
        status_t ret = rpmsg_channel_recv(rpmsg_chan, msg_buf, SDSHELL_MAX_RPMSG_SIZE, &msg_len, 100);
        if (ret) {
            continue;
        }

        sdshell_event_handle(ctl, msg_buf, msg_len, 0, NULL);
    }
}

static void sdshell_task(void *arg)
{
    sdshell_ctl_t *ctl = (sdshell_ctl_t *)arg;
    int len;

    if (!ctl) {
        ssdk_printf(SSDK_CRIT, "%s: sdshell_task: ctl is NULL\n",  __func__);
        return;
    }

    for (;;) {
        ctl->thread_running = 0;
        osEventFlagsWait(ctl->event, 1, osFlagsWaitAny, osWaitForever);
        ctl->thread_running = 1;
        do {
            len = virt_uart_get_new_buf(vuart_tx_buf, LINE_TXBUF_SIZE);

            if (len <= 0)
                break;

            sdshell_send_msg(ctl, (char *)vuart_tx_buf, len);
        } while (len >= LINE_TXBUF_SIZE);
    }
}

static void sdshell_console_entry(int index)
{
    if (index < 0 || index >= sdshell_ctrl_idx_max)
        return;

    const char *sdshell_name = "sdshell_tx";
    sdshell_ctl_t *ctl = sdshell_ctl_get(index);
    osThreadAttr_t sdshell_task_attr = {0};

    ctl->dev = rpmsg_get_instance(RPMSG_IPCC, ctl->rproc);
    if (!ctl->dev) {
        ssdk_printf(SSDK_ALERT, "rpmsg_get_instance failed\n");
        return;
    }

    ctl->chan = rpmsg_channel_create(ctl->dev, ctl->chan_name, ctl->ept_id);
    if (ctl->chan) {
        rpmsg_channel_start(ctl->chan, NULL, NULL);
    }
    else {
        ssdk_printf(SSDK_ALERT, "%s: failed to create ipcc channel for sdshell\n", __func__);
        return;
    }

    ctl->event = osEventFlagsNew(NULL);
    if (ctl->event == NULL) {
            ssdk_printf(SSDK_ALERT, "%s: create event failed.\n", __func__);
            return;
     }

    sdshell_task_attr.name       = sdshell_name;
    sdshell_task_attr.stack_size = SDSHELL_TX_STACK_SIZE;
    sdshell_task_attr.priority   = osPriorityNormal;
    ctl->thread = osThreadNew(sdshell_task, (void *)ctl, &sdshell_task_attr);
    if (ctl->thread == NULL) {
        ssdk_printf(SSDK_ALERT, "%s: create thread failed.\n", __func__);
        osEventFlagsDelete(ctl->event);
        ctl->event = NULL;
        return;
    }

    osThreadAttr_t recv_attr = { .name = "sdshell_recv", .stack_size = 4096 };
    ctl->recv_thread = osThreadNew(sdshell_recv_task, (void*)ctl, &recv_attr);
    if (ctl->recv_thread == NULL) {
        ssdk_printf(SSDK_ALERT, "%s: create recv_thread failed.\n", __func__);
        return;
    }

    /* core type init */
    ctl->core_id = SDRV_CORE_MAX;

    ssdk_printf(SSDK_ALERT, "sdshell init succeeded\n");
}

void sdshell_console_init(void)
{
#if CONFIG_VIRT_CONSOLE_AP1_BE
    sdshell_console_entry(sdshell_ctrl_idx_ap1);
#endif
#if CONFIG_VIRT_CONSOLE_MP_BE
    sdshell_console_entry(sdshell_ctrl_idx_mp);
#endif
}
