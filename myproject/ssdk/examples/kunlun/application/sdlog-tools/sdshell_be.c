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
#include <driver/kunlun/virt_uart/logbuf.h>
#include <param.h>
#include <board.h>
#include <driver/uart.h>
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#include <rpc/rpmsg_common.h>
#ifdef CONFIG_SDRV_VIRT_CONSOLE
#include <sdshell.h>
#endif
#include "sdshell_be.h"

#define SDSHELL_LOGE(fmt, ...) ssdk_printf(SSDK_ERR,    "[SDSHELL:E][%s]" fmt "\n", __func__, ##__VA_ARGS__)
#define SDSHELL_LOGW(fmt, ...) ssdk_printf(SSDK_WARNING,"[SDSHELL:W][%s]" fmt "\n", __func__, ##__VA_ARGS__)
#define SDSHELL_LOGI(fmt, ...) ssdk_printf(SSDK_NOTICE, "[SDSHELL:I][%s]" fmt "\n", __func__, ##__VA_ARGS__)
#define SDSHELL_LOGD(fmt, ...) ssdk_printf(SSDK_DEBUG,  "[SDSHELL:D][%s]" fmt "\n", __func__, ##__VA_ARGS__)

#define SDSHELL_MAX_RPMSG_SIZE  (512)
#define SDSHELL_MAX_INPUT_SIZE  (512)
#define SDSHELL_PROMPT_LEN      (32)
#define SDSHELL_CONSOLE         board_get_console()
#define SDSHELL_LOGBUF_MAX_SIZE (2048)

#define BIT_MASK(n)             (1<<(n))
#define R_CORE_TYPE_MASK        (BIT_MASK(SAFETY_R5)|BIT_MASK(SECURE_R5))
#define A_CORE_TYPE_MASK        (BIT_MASK(AP1_A55)|BIT_MASK(AP2_A55))
#define ALL_CORE_TYPE_MASK      (R_CORE_TYPE_MASK|A_CORE_TYPE_MASK)

#define IS_R_CORE(type) \
        ({ \
            uint8_t r_core = 0; \
            if (((type) == SAFETY_R5) \
                || ((type) == SECURE_R5) \
                || ((type) == MPC_R5)) { \
                r_core = 1; \
            } \
            r_core; \
        })

#define IS_REMOTE_CMD(cmd) \
        ({ \
            uint8_t is_remote = 0; \
            if (((cmd) == SDSHELL_MSG_QUIT) \
                || ((cmd) == SDSHELL_MSG_PRINTON) \
                || ((cmd) == SDSHELL_MSG_PRINTOFF)) { \
                is_remote = 1; \
            } \
            is_remote; \
        })

#ifndef SDSHELL_SERVICE_NAME
#define SDSHELL_SERVICE_NAME    "sdshell-service"
#endif
#ifndef SDSHELL_MP_NAME
#define SDSHELL_MP_NAME         "sdshell-mp"
#endif

#ifndef CONFIG_SDRV_VIRT_CONSOLE
typedef enum {
    SDSHELL_MSG_READ = 1,
    SDSHELL_MSG_CLEAR,
    SDSHELL_MSG_RUN_CMD,
    SDSHELL_MSG_TARGET_SET,
    SDSHELL_MSG_QUIT,
    SDSHELL_MSG_PRINTON,
    SDSHELL_MSG_PRINTOFF,
    SDSHELL_MSG_RUN_CMD_FEEDBACK, /* frontend -> backend */
    SDSHELL_MSG_MAX,
} msg_type_t;

typedef struct _sdshell_msg_t {
    uint32_t type;
    uint32_t size;
    uint8_t data[0];
} sdshell_msg_t;
#endif

typedef struct _sdshell_dev_st {
    const char* name;
    sdrv_mbox_rproc_t rproc;
    core_type_t core_id;
    rpmsg_channel_t chan;
    struct rpmsg_instance* inst;
    osThreadId_t recv_thread;
    bool is_running;
    bool xcon_en;
    bool xcon_pause;
} sdshell_dev_st;

typedef struct _sdshell_input_st {
    char* buff;
    uint16_t pos;
} sdshell_input_st;

typedef struct _sdshell_cmd_cfg_st {
    char* cmd;
    int type;
    int send_mask;
} sdshell_cmd_cfg_st;

sdshell_dev_st sdshell_dev[] = {
    {
        .name = "safety",
        .rproc = MBOX_RPROC_SAF,
        .core_id = SAFETY_R5,
        .inst = NULL,
        .chan = NULL,
        .recv_thread = NULL,
        .is_running = false,
        .xcon_en = false,
    },
    {
        .name = "secure",
        .rproc = MBOX_RPROC_SEC,
        .core_id = SECURE_R5,
        .inst = NULL,
        .chan = NULL,
        .recv_thread = NULL,
        .is_running = false,
        .xcon_en = false,
    },
    {
        .name = "ap1",
        .rproc = MBOX_RPROC_AP1,
        .core_id = AP1_A55,
        .inst = NULL,
        .chan = NULL,
        .recv_thread = NULL,
        .is_running = false,
        .xcon_en = false,
    },
    {
        .name = "ap2",
        .rproc = MBOX_RPROC_AP2,
        .core_id = AP2_A55,
        .inst = NULL,
        .chan = NULL,
        .recv_thread = NULL,
        .is_running = false,
        .xcon_en = false,
    },
};

static const sdshell_cmd_cfg_st sdshell_cmd_cfgs[] = {
    { .cmd = "quit",     .type = SDSHELL_MSG_QUIT },
    { .cmd = "exit",     .type = SDSHELL_MSG_QUIT },
    { .cmd = "logshow",  .type = SDSHELL_MSG_READ },
    { .cmd = "logclear", .type = SDSHELL_MSG_CLEAR },
    { .cmd = "xon",      .type = SDSHELL_MSG_PRINTON },
    { .cmd = "xoff",     .type = SDSHELL_MSG_PRINTOFF },
};

sdshell_input_st sdshell_input;
bool sdshell_running = false;

static void sdshell_recv_r5_logs(sdshell_dev_st* dev, char* buff, int len)
{
    if (!buff)
        return;

    for (int i = 0; i < len; i++) {
        board_raw_putc(buff[i]);
        if (buff[i] == '\n') {
            for (int j = 0; j < strlen(dev->name); j++) {
                board_raw_putc(dev->name[j]);
            }
            board_raw_putc(' ');
            board_raw_putc(']');
        }
    }
}

static void sdshell_recv_a55_logs(sdshell_dev_st* dev, char* buff, int len)
{
    if (!dev || !buff)
        return;

    sdshell_msg_t* msg = (sdshell_msg_t*)buff;
    int output_len = 0;
    char* output_buf = malloc(SDSHELL_LOGBUF_MAX_SIZE);
    if (!output_buf) {
        SDSHELL_LOGE("malloc output_buf fail");
        return;
    }

    if (msg->type == SDSHELL_MSG_RUN_CMD_FEEDBACK) {
        uint32_t mask = 0;
        memcpy((char*)&mask, msg->data, msg->size);
        dev->xcon_en = true;
        SDSHELL_LOGD("recv feedback, mask:%x", mask);
        for (int i = 0; i < SDRV_CORE_MAX; i++, mask>>=1) {
            if (mask&1) {
                void* hdr = sdrv_logbuf_get_bank_hdr(i);
                if (hdr) {
                    memset(output_buf, 0, SDSHELL_LOGBUF_MAX_SIZE);
                    output_len = sdrv_logbuf_get_new_buf(hdr, output_buf, SDSHELL_LOGBUF_MAX_SIZE);
                    for (int j = 0; j < output_len; j++) {
                        board_raw_putc(output_buf[j]);
                        if (output_buf[j] == '\n') {
                            for (int k = 0; k < strlen(dev->name); k++) {
                                board_raw_putc(dev->name[k]);
                            }
                            board_raw_putc(' ');
                            board_raw_putc(']');
                        }
                    }
                }
            }
        }
    }

    if (output_buf) {
        free(output_buf);
        output_buf = NULL;
    }
}

static void sdshell_recv_logs(void* arg)
{
    sdshell_dev_st* dev = NULL;
    struct rpmsg_channel* rpmsg_chan = NULL;
    char* msg_buf = NULL;
    int msg_len = 0;
    status_t ret = 0;

    dev = (sdshell_dev_st*)arg;
    if (!dev) {
        SDSHELL_LOGE("dev is NULL");
        goto exit;
    }

    rpmsg_chan = (struct rpmsg_channel*)dev->chan;
    if (!rpmsg_chan) {
        SDSHELL_LOGE("rpmsg channel is NULL");
        goto exit;
    }

    msg_buf = malloc(SDSHELL_MAX_RPMSG_SIZE);
    if (!msg_buf) {
        SDSHELL_LOGE("malloc msg_buf fail");
        goto exit;
    }

    while (dev->is_running) {
        msg_len = 0;
        memset(msg_buf, 0, SDSHELL_MAX_RPMSG_SIZE);
        ret = rpmsg_channel_recv(rpmsg_chan, msg_buf, SDSHELL_MAX_RPMSG_SIZE, &msg_len, 100);
        if (ret) {
            SDSHELL_LOGD("recv msg timeout.");
            continue;
        }

        if (IS_R_CORE(dev->core_id)) {
            sdshell_recv_r5_logs(dev, msg_buf, msg_len);
        }
        else {
            sdshell_recv_a55_logs(dev, msg_buf, msg_len);
        }
    }

exit:
    if (msg_buf) {
        free(msg_buf);
        msg_buf = NULL;
    }

    osThreadExit();
}

static int sdshell_send_cmd(rpmsg_channel_t chan, int msg_type, char* data, int len)
{
    if (!chan)
        return -1;

    int size = sizeof(sdshell_msg_t) + len;
    sdshell_msg_t* msg = (sdshell_msg_t*)malloc(size);

    if (!msg) {
        SDSHELL_LOGE("failed to malloc memory for msg");
        return -1;
    }

    memset((void*)msg, 0, size);
    msg->type = msg_type;
    msg->size = len;
    if (data) {
        memcpy((char*)msg->data, data, len);
    }

    status_t ret = rpmsg_channel_send(chan, SDSHELL_MP_EPT, (void*)msg, size, 1000);
    if (ret)
        SDSHELL_LOGE("send comamnd fail.");


    if (msg) {
        free(msg);
        msg = NULL;
    }

    return 0;
}

static void sdshell_read_logbuf(const char* domain, char** buff, uint32_t* size, uint32_t len)
{
    if (!domain || !buff || !size)
        return;

    for (int i = 0; i < len; i++) {
        if (size[i] > 0) {
            for (int j = 0; j < size[i]; j++) {
                board_raw_putc(buff[i][j]);
                if (buff[i][j] == '\n') {
                    for (int k = 0; k < strlen(domain); k++) {
                        board_raw_putc(domain[k]);
                    }
                    if (!(i+1 >= len && j+1 >= size[i])) {
                        board_raw_putc(' ');
                        board_raw_putc(']');
                    }
                }
            }
        }
    }
}

static int sdshell_local_cmd(sdshell_dev_st* dev, int msg_type)
{
    if (!dev || IS_R_CORE(dev->core_id))
        return -1;

    switch (msg_type) {
    case SDSHELL_MSG_READ:
        {
            /* kernel */
            char* buff[SDRV_LOGBUG_SEGMENT_NUM] = {0};
            uint32_t size[SDRV_LOGBUG_SEGMENT_NUM] = {0};
            void* hdr_kernel = sdrv_logbuf_get_bank_hdr(dev->core_id);
            void* hdr_user = sdrv_logbuf_get_bank_hdr(dev->core_id+1);
            int ret = sdrv_logbuf_get_last_buf_and_size(hdr_kernel, buff, size, SDRV_LOGBUG_SEGMENT_NUM);
            if (!ret)
                sdshell_read_logbuf(dev->name, buff, size, SDRV_LOGBUG_SEGMENT_NUM);

            /* user */
            memset(buff, 0, sizeof(buff));
            memset(size, 0, sizeof(size));
            ret = sdrv_logbuf_get_last_buf_and_size(hdr_user, buff, size, SDRV_LOGBUG_SEGMENT_NUM);
            if (!ret)
                sdshell_read_logbuf(dev->name, buff, size, SDRV_LOGBUG_SEGMENT_NUM);
            break;
        }
    case SDSHELL_MSG_CLEAR:
        {
            void* hdr_kernel = sdrv_logbuf_get_bank_hdr(dev->core_id);
            void* hdr_user = sdrv_logbuf_get_bank_hdr(dev->core_id+1);
            sdrv_logbuf_clear(hdr_kernel);
            sdrv_logbuf_clear(hdr_user);
            break;
        }
    case SDSHELL_MSG_PRINTON:
        {
            dev->xcon_en = true;
            break;
        }
    case SDSHELL_MSG_PRINTOFF:
        {
            dev->xcon_en = false;
            break;
        }
    default:
        {
            break;
        }
    }

    return 0;
}

static int sdshell_cmd_parser(sdshell_dev_st* dev, char* cmd, uint16_t len)
{
    if (!dev || !cmd)
        return 0;

    if (len == 0)
        return 0;

    int ret = 0;
    bool found_cmd = false;
    for (int i = 0; i < ARRAY_SIZE(sdshell_cmd_cfgs); i++) {
        sdshell_cmd_cfg_st* cfg = &sdshell_cmd_cfgs[i];
        if (!strcasecmp(cmd, cfg->cmd)) {
            found_cmd = true;
            if (IS_R_CORE(dev->core_id) || IS_REMOTE_CMD(cfg->type)) {
                sdshell_send_cmd(dev->chan, cfg->type, NULL, 0);
                if (cfg->type == SDSHELL_MSG_QUIT) {
                    dev->xcon_en = false;
                    osDelay(1000);
                    ret = -1; // quit
                }
            }
            else {
                sdshell_local_cmd(dev, cfg->type);
            }
            break;
        }
    }

    if (!found_cmd) {
        sdshell_send_cmd(dev->chan, SDSHELL_MSG_RUN_CMD, cmd, len);
    }

    return ret;
}

static int sdshell_uart_handle(sdshell_dev_st* dev, const char* prompt, size_t prompt_len)
{
    if (!dev)
        return false;

    int ret = 0;
    uart_dev_t* console = SDSHELL_CONSOLE;
    char rxchar = 0;

    uart_receive(console, &rxchar, 1);
    switch (rxchar) {
    case '\r':
    case '\n':
        {
            uart_raw_transmit(console, "\r\n", 2);
            if (prompt)
                uart_raw_transmit(console, prompt, prompt_len);
            else
                uart_raw_transmit(console, (const char*)"] ", 2);
            ret = sdshell_cmd_parser(dev, sdshell_input.buff, sdshell_input.pos);
            memset(sdshell_input.buff, 0, SDSHELL_MAX_INPUT_SIZE);
            sdshell_input.pos = 0;
            break;
        }

    case 0x08: // bs
    case 0x7f: // del
        {
            char bs = 0x08;
            char space = ' ';
            uart_raw_transmit(console, &bs, 1);
            uart_raw_transmit(console, &space, 1);
            uart_raw_transmit(console, &bs, 1);
            if (sdshell_input.buff) {
                if (sdshell_input.pos > 0 && sdshell_input.pos < SDSHELL_MAX_INPUT_SIZE) {
                    sdshell_input.buff[sdshell_input.pos-1] = 0;
                    sdshell_input.pos -= 1;
                }
            }
            break;
        }

    default:
        {
            uart_raw_transmit(console, &rxchar, 1);
            if (sdshell_input.buff && sdshell_input.pos < SDSHELL_MAX_INPUT_SIZE)
                sdshell_input.buff[sdshell_input.pos++] = rxchar;
            break;
        }
    }

    return ret;
}

static void sdshell_uart_rx_loop(sdshell_dev_st* dev)
{
    if (!dev)
        return;

    char prompt[SDSHELL_PROMPT_LEN] = {0};
    uart_dev_t* console = SDSHELL_CONSOLE;

    memset(&sdshell_input, 0, sizeof(sdshell_input));
    sdshell_input.buff = (char*)malloc(SDSHELL_MAX_INPUT_SIZE);
    if (!sdshell_input.buff)
        return;

    snprintf(prompt, SDSHELL_PROMPT_LEN, "%s ] ", dev->name);
    uart_raw_transmit(console, prompt, strlen(prompt));

    for (;;) {
        if (sdshell_uart_handle(dev, prompt, strlen(prompt))) {
            dev->is_running = false;
            break;
        }
    }

    if (sdshell_input.buff) {
        free(sdshell_input.buff);
        sdshell_input.buff = NULL;
        sdshell_input.pos = 0;
    }
}

static void sdshell_connect(sdshell_dev_st* dev)
{
    if (!dev) {
        SDSHELL_LOGE("dev not found.");
        return;
    }

    dev->inst = rpmsg_get_instance(RPMSG_TYPE_IPCC, dev->rproc);
    if (!dev->inst) {
        SDSHELL_LOGE("get rproc:%d rpmsg instance fail.", dev->rproc);
        return;
    }

    dev->chan = rpmsg_channel_create(dev->inst, SDSHELL_MP_NAME, SDSHELL_MP_EPT);
    if (!dev->chan) {
        SDSHELL_LOGE("ept create fail.");
        return;
    }

    status_t ret = rpmsg_channel_start(dev->chan, NULL, NULL);
    if (ret) {
        SDSHELL_LOGE("rpmsg channel start fail.");
        return;
    }
}

static void sdshell_entry(sdshell_dev_st* dev)
{
    if (dev && dev->chan) {
        dev->is_running = true;

        osThreadAttr_t attr_recv = { .name = "sdshell_be_recv" };
        dev->recv_thread = osThreadNew(sdshell_recv_logs, (void*)(dev), &attr_recv);

        sdshell_send_cmd(dev->chan, SDSHELL_MSG_TARGET_SET, (char*)&dev->core_id, sizeof(dev->core_id));
        sdshell_uart_rx_loop(dev);
    }
}

bool is_sdshell_backend_running(void)
{
    return sdshell_running;
}

void board_sdshell_be_init(void)
{
    for (int i = 0; i < ARRAY_SIZE(sdshell_dev); i++) {
        sdshell_dev_st* dev = NULL;
#if CONFIG_SDSHELL_AP1_FRONTEND
        if (!strcasecmp("ap1", sdshell_dev[i].name)) {
            dev = &sdshell_dev[i];
        }
#endif
#if CONFIG_SDSHELL_AP2_FRONTEND
        if (!strcasecmp("ap2", sdshell_dev[i].name)) {
            dev = &sdshell_dev[i];
        }
#endif
        if (dev && !dev->chan)
            sdshell_connect(dev);
    }
}

static int cmd_sdshell(int argc, char* argv[])
{
    if (argc != 1)
        return 0;

    char* domain = argv[0];
    for (int i = 0; i < ARRAY_SIZE(sdshell_dev); i++) {
        if (!strcasecmp(domain, sdshell_dev[i].name)) {
            sdshell_running = true;
            sdshell_dev_st* dev = &sdshell_dev[i];
            if (!dev->chan) {
                sdshell_connect(dev);
            }
            sdshell_entry(dev);
            break;
        }
    }

    sdshell_running = false;
    return 0;
}

CLI_CMD("sdshell", "sdshell [safety|secure|ap1|ap2]\n", cmd_sdshell);
/* EOF */
