/**
 * @file sdrv_sent_test.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <board.h>
#include <reg.h>
#include <param.h>
#include <CLI_type.h>
#include <driver/taishan/sent/sdrv_sent_drv.h>
#include <sdrv_xtrg.h>
#include <driver/taishan/xtrg/sdrv_xtrg_drv.h>
#include <regs_base.h>
#include <driver/sent.h>
#include <sent_cfg.h>
#include <debug.h>

#define SDRV_SENT_DEMO_ALWAYS(format, ...) \
    ssdk_printf(SSDK_EMERG, format, ##__VA_ARGS__)
#define SDRV_SENT_DEMO_ERR(format, ...) \
    ssdk_printf(SSDK_ERR, format, ##__VA_ARGS__)
#define SDRV_SENT_DEMO_WARN(format, ...) \
    ssdk_printf(SSDK_WARNING, format, ##__VA_ARGS__)
#define SDRV_SENT_DEMO_INFO(format, ...) \
    ssdk_printf(SSDK_INFO, format, ##__VA_ARGS__)

#ifdef CONFIG_BOARD_D3_GATEWAY
#define SENT_DETECT_PORT  11 //GPIO_A11 Xtrg_11
// #define SENT_DETECT_PORT  25 //GPIO_S7 Xtrg_25
#endif  /* CONFIG_BOARD_E3_GATEWAY */
#ifdef CONFIG_BOARD_E3_GATEWAY
#define SENT_DETECT_PORT  11 //GPIO_A11 Xtrg_11
// #define SENT_DETECT_PORT  25 //GPIO_S7 Xtrg_25
#endif  /* CONFIG_BOARD_E3_GATEWAY */
#ifdef CONFIG_BOARD_E3_176_REF
#define SENT_DETECT_PORT  16 //GPIO_B0 Xtrg_16
#endif  /* CONFIG_BOARD_E3_176_REF */
#ifdef CONFIG_BOARD_E3_144_REF
#define SENT_DETECT_PORT  16 //GPIO_B0 Xtrg_16
#endif  /* CONFIG_BOARD_E3_144_REF */

xtrg_config_t xtrg_sent_config;

xtrg_dev_t xtrg_dev_sent_hfre = {
    .base = APB_XTRG_BASE,
    .irq = XTRG_FUNC_INTR_NUM,
};

static sent_dev_t *sent_get_dev(uint8_t sent_id)
{
    for (uint32_t cnt = 0; cnt < SENT_DEV_NUM; cnt++) {
        sent_dev_t *sent_dev = g_sent[cnt];
        sdrv_sent_dev_t *sdrv_sent_dev = sent_dev->priv;
        if (sent_dev != NULL && sdrv_sent_dev->hwid == sent_id) {
            return sent_dev;
        }
    }
    return NULL;
}

static void sent_dev_list(void)
{
    SDRV_SENT_DEMO_ALWAYS("SENT Dev List:\n");
    for (uint32_t cnt = 0; cnt < SENT_DEV_NUM; cnt++) {
        sent_dev_t *sent_dev = g_sent[cnt];
        sdrv_sent_dev_t *sdrv_sent_dev = sent_dev->priv;
        if (sent_dev != NULL) {
            SDRV_SENT_DEMO_ALWAYS("SENT %d is availabe\n", sdrv_sent_dev->hwid);
        }
    }
}

static void sent_recvA_test_cb(void *arg)
{
    sent_dev_t *sent_dev = arg;
    static uint32_t frame = 0;
    uint32_t fraSta = 0;
    uint8_t status = 0;
    uint8_t data[6] = {0};

    fraSta = sent_get_frame(sent_dev, SENT_HW_CPT_A, data, &status);
    printf("0x%x, 0x%x, %d, %d %d %d %d %d %d\n",
        frame ++, fraSta, status, data[0], data[1], data[2], data[3], data[4], data[5]);
}

static void sent_recvB_test_cb(void *arg)
{
    sent_dev_t *sent_dev = arg;
    static uint32_t frame = 0;
    uint32_t fraSta = 0;
    uint8_t status = 0;
    uint8_t data[6] = {0};

    fraSta = sent_get_frame(sent_dev, SENT_HW_CPT_B, data, &status);
    printf("0x%x, 0x%x, %d, %d %d %d %d %d %d\n",
        frame ++, fraSta, status, data[0], data[1], data[2], data[3], data[4], data[5]);
}

static void sent_init_set_callback(sent_dev_t *sent_dev, uint8_t sent_chanid)
{
    switch (sent_chanid){
        case SENT_HW_CPT_A:
            sent_set_callback(sent_dev, SENT_HW_CPT_A, sent_recvA_test_cb, sent_dev);
            break;
        case SENT_HW_CPT_B:
            sent_set_callback(sent_dev, SENT_HW_CPT_B, sent_recvB_test_cb, sent_dev);
            break;
        default:
            break;
    }
}

static void xtag_config(uint8_t xtag_port, uint8_t sent_id, uint8_t sent_chanid)
{
    memset(&xtrg_sent_config, 0, sizeof(xtrg_sent_config));

    xtrg_sent_config.sig_i = IO_i(xtag_port);
    switch (sent_chanid)
    {
        case SENT_HW_CPT_A:
        {
            xtrg_sent_config.sig_o = ETIMER_CPT0((sent_id -1), 0);
            break;
        }
        case SENT_HW_CPT_B:
        {
            xtrg_sent_config.sig_o = ETIMER_CPT0((sent_id -1), 2);
            break;
        }
    }

    sdrv_xtrg_smux_config(&xtrg_dev_sent_hfre, &xtrg_sent_config);
}

static int sent_init_demo(int argc, char *argv[])
{
    SDRV_SENT_DEMO_ALWAYS("sent_init_demo : sent init. \n");

    sent_dev_t *sent_dev;
    uint8_t sent_id;

    if (argc != 1) {
        SDRV_SENT_DEMO_ERR("argc must be 1\n");
        return -1;
    }

    sent_id = atoi(argv[0]);
    sent_dev = sent_get_dev(sent_id);
    if (!sent_dev) {
        SDRV_SENT_DEMO_ERR("sent id error\n");
        return -1;
    }
    sent_init(sent_dev);
    sent_init_set_callback(sent_dev, sent_id);

    return 0;
}

static int sent_deinit_demo(int argc, char *argv[])
{
    SDRV_SENT_DEMO_ALWAYS("sent_deinit_demo : sent deinit. \n");
    sent_dev_t *sent_dev;
    uint8_t sent_id;

    if (argc != 1) {
        SDRV_SENT_DEMO_ERR("argc must be 1\n");
        return -1;
    }

    sent_id = atoi(argv[0]);
    sent_dev = sent_get_dev(sent_id);
    if (!sent_dev) {
        SDRV_SENT_DEMO_ERR("sent id error\n");
        return -1;
    }

    sent_deinit(sent_dev);

    return 0;
}

static int sent_recv_start_demo(int argc, char *argv[])
{
    SDRV_SENT_DEMO_ALWAYS("sent_recv_start_demo : sent start recv frame.\n");

    sent_dev_t *sent_dev;
    uint8_t sent_id;
    uint8_t sent_chanid;

    if (argc != 2) {
        SDRV_SENT_DEMO_ERR("argc must be 2\n");
        return -1;
    }

    sent_id = atoi(argv[0]);
    sent_chanid = atoi(argv[1]);
    sent_dev = sent_get_dev(sent_id);
    if (!sent_dev) {
        SDRV_SENT_DEMO_ERR("sent id error\n");
        return -1;
    }

    sent_init(sent_dev);
    sent_init_set_callback(sent_dev, sent_chanid);
    xtag_config(SENT_DETECT_PORT, sent_id, sent_chanid);
    sent_recv_start(sent_dev, sent_chanid);

    return 0;
}

static int sent_recv_stop_demo(int argc, char *argv[])
{
    SDRV_SENT_DEMO_ALWAYS("sent_recv_stop_demo : sent stop recv frame.\n");
    sent_dev_t *sent_dev;
    uint8_t sent_id;
    uint8_t sent_chanid;

    if (argc != 2) {
        SDRV_SENT_DEMO_ERR("argc must be 2\n");
        return -1;
    }

    sent_id = atoi(argv[0]);
    sent_chanid = atoi(argv[1]);
    sent_dev = sent_get_dev(sent_id);
    if (!sent_dev) {
        SDRV_SENT_DEMO_ERR("sent id error\n");
        return -1;
    }

    sent_recv_stop(sent_dev, sent_chanid);

    return 0;
}

static void sent_test_usage(void)
{
    SDRV_SENT_DEMO_ALWAYS("\n");

    SDRV_SENT_DEMO_ALWAYS("sent_demo ls");
    SDRV_SENT_DEMO_ALWAYS("\t:sent dev list\n");

    SDRV_SENT_DEMO_ALWAYS("sent_demo init [sent_id]");
    SDRV_SENT_DEMO_ALWAYS("\t:sent moudle and channel init, sent_id refer to dev list .\n");

    SDRV_SENT_DEMO_ALWAYS("sent_demo deinit [sent_id]");
    SDRV_SENT_DEMO_ALWAYS("\t:sent moudle and channel deinit, sent_id refer to dev list .\n");

    SDRV_SENT_DEMO_ALWAYS("sent_demo recv_start [sent_id][chan_id]");
    SDRV_SENT_DEMO_ALWAYS("\t:sent recv start, sent_id refer to dev list, chan_id is 0 ~ 1 .\n");

    SDRV_SENT_DEMO_ALWAYS("sent_demo recv_stop [sent_id][chan_id]");
    SDRV_SENT_DEMO_ALWAYS("\t:sent recv stop, sent_id refer to dev list, chan_id is 0 ~ 1 .\n");
}

static int sent_demo(int argc, char *argv[])
{
    if (argc == 0) {
        sent_test_usage();
        return CLI_FALSE;
    }
    else if (!strcmp(argv[0], "ls")) {
        sent_dev_list();
    }
    else if (!strcmp(argv[0], "init")) {
        sent_init_demo(argc - 1, &argv[1]);
    }
    else if (!strcmp(argv[0], "deinit")) {
        sent_deinit_demo(argc - 1, &argv[1]);
    }
    else if (!strcmp(argv[0], "recv_start")) {
        sent_recv_start_demo(argc - 1, &argv[1]);
    }
    else if (!strcmp(argv[0], "recv_stop")) {
        sent_recv_stop_demo(argc - 1, &argv[1]);
    }
    else {
        SDRV_SENT_DEMO_ERR("cmd error:%s\n", argv[0]);
    }

    return CLI_FALSE;
}

CLI_CMD("sent_demo", "\r\nsent_demo:\r\n sent demo\r\n", sent_demo);
