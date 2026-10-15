/**
 * @file lin_demo.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: lin demo application.
 *
 * Revision History:
 * -----------------
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <driver/taihang/sdrv_lin/sdrv_lin_drv.h>
#include <CLI_type.h>
#include <lin_cfg.h>
#include <board.h>
#include <debug.h>
#include <clock_cfg.h>

#define SDRV_LIN_DEMO_ALWAYS(format, ...)                                      \
    ssdk_printf(SSDK_EMERG, format, ##__VA_ARGS__)
#define SDRV_LIN_DEMO_ERR(format, ...)                                         \
    ssdk_printf(SSDK_ERR, format, ##__VA_ARGS__)
#define SDRV_LIN_DEMO_WARN(format, ...)                                        \
    ssdk_printf(SSDK_WARNING, format, ##__VA_ARGS__)
#define SDRV_LIN_DEMO_INFO(format, ...)                                        \
    ssdk_printf(SSDK_INFO, format, ##__VA_ARGS__)

#define TIMEOUT_US 100000

const uint8_t test_message[8] = {1, 2, 3, 4, 5, 6, 7, 8};
/* test result count */
uint32_t lin_fail = 0;
uint32_t lin_success = 0;
uint32_t summary_count = 0;

static lin_dev_t *lin_get_dev(uint32_t hwid)
{
    for (uint32_t cnt = 0; cnt < LIN_DEV_NUM; cnt++) {
        lin_dev_t *lin_dev = g_lin_dev[cnt];
        if (lin_dev != NULL && lin_dev->hwid == hwid) {
            return lin_dev;
        }
    }
    return NULL;
}

static int lin_dev_list(int argc, char *argv[])
{
    SDRV_LIN_DEMO_ALWAYS("LIN Dev List:\n");
    for (uint32_t cnt = 0; cnt < LIN_DEV_NUM; cnt++) {
        lin_dev_t *lin_dev = g_lin_dev[cnt];
        if (lin_dev != NULL) {
            SDRV_LIN_DEMO_ALWAYS("LIN %d is availabe\n", lin_dev->hwid);
        }
    }
    return 0;
}

static int lin_wakeup_internal_demo(int argc, char *argv[])
{
    SDRV_LIN_DEMO_ALWAYS(" lin_wakeup_internal_demo start!\n");

    lin_dev_t *lin_dev;
    uint8_t lin_id;

    if (argc != 1) {
        SDRV_LIN_DEMO_ERR(" argc must be 1\n");
        return -1;
    }

    lin_id = atoi(argv[0]);
    lin_dev = lin_get_dev(lin_id);
    if (!lin_dev) {
        SDRV_LIN_DEMO_ERR(" lin id error\n");
        return -1;
    }

    lin_wakeup_internal(lin_dev);
    SDRV_LIN_DEMO_ALWAYS(" lin_wakeup_internal_demo success!\n");
    return 0;
}

static int lin_send_frame_demo(int argc, char *argv[])
{
    SDRV_LIN_DEMO_ALWAYS(" lin_send_frame_demo start!\n");

    lin_dev_t *lin_dev;
    lin_pdu_t pdu = {0};
    uint8_t lin_id;
    int ret = -1;

    if (argc != 2) {
        SDRV_LIN_DEMO_ERR(" argc must be 2\n");
        goto send_out;
    }

    lin_id = atoi(argv[0]);
    lin_dev = lin_get_dev(lin_id);
    if (!lin_dev) {
        SDRV_LIN_DEMO_ERR(" lin id error\n");
        goto send_out;
    }

    pdu.pid = atoi(argv[1]);
    pdu.cs = LIN_ENHANCED_CS;
    pdu.drc = LIN_MASTER_RESPONSE;
    pdu.dl = 8;

    pdu.sdu = (uint8_t *)osAlloc(pdu.dl);
    if (pdu.sdu == NULL) {
        SDRV_LIN_DEMO_ERR(" malloc sdu fail\n");
        goto send_out;
    }
    memset(pdu.sdu, 0x11, pdu.dl);

    if (lin_send_frame(lin_dev, &pdu) != LIN_OK) {
        SDRV_LIN_DEMO_ERR(" lin send frame error\n");
        goto send_out_with_mem;
    }

    SDRV_LIN_DEMO_ALWAYS(" lin tx data send success!\n");
    ret = 0;

send_out_with_mem:
    osFree(pdu.sdu);

send_out:
    return ret;
}

static int lin_recv_frame_demo(int argc, char *argv[])
{
    SDRV_LIN_DEMO_ALWAYS(" lin_recv_frame_demo start!\n");

    lin_dev_t *lin_dev;
    lin_pdu_t pdu = {0};
    lin_status_t status;
    uint8_t lin_id;
    uint8_t *rx_sdu;

    if (argc != 2) {
        SDRV_LIN_DEMO_ERR(" argc must be 2\n");
        return -1;
    }

    lin_id = atoi(argv[0]);
    lin_dev = lin_get_dev(lin_id);
    if (!lin_dev) {
        SDRV_LIN_DEMO_ERR(" lin id error\n");
        return -1;
    }

    pdu.pid = atoi(argv[1]);
    pdu.cs = LIN_ENHANCED_CS;
    pdu.drc = LIN_SLAVE_RESPONSE;
    pdu.dl = 8;
    pdu.sdu = NULL;

    if (lin_send_frame(lin_dev, &pdu) != LIN_OK) {
        SDRV_LIN_DEMO_ERR(" lin send frame error\n");
        return -1;
    }

    while ((status = lin_get_status(lin_dev, &rx_sdu)) != LIN_RX_OK) {
        osDelay(100);
    }

    SDRV_LIN_DEMO_ALWAYS(" get_status:%d\n", status);
    SDRV_LIN_DEMO_ALWAYS(" lin rx data:");
    for (uint8_t cnt = 0; cnt < LIN_FRAME_DATA_LENGTH - 1; cnt++) {
        SDRV_LIN_DEMO_ALWAYS("0x%x ", rx_sdu[cnt]);
    }
    SDRV_LIN_DEMO_ALWAYS("\n");
    SDRV_LIN_DEMO_ALWAYS(" lin_recv_frame_demo success!\n");
    return 0;
}

static int lin_stress_test(int argc, char *argv[])
{
    SDRV_LIN_DEMO_ALWAYS(" lin_stress_test start!\n");
    if (argc != 3) {
        SDRV_LIN_DEMO_ERR(" argc must be 3\n");
        return -1;
    }

    lin_dev_t *lin_dev;
    lin_pdu_t transmit_pdu = {0};
    lin_pdu_t receive_pdu = {0};
    uint8_t lin_id;
    uint8_t *rx_sdu;
    uint8_t *tx_sdu;
    volatile uint32_t transfer_count = 0;

    lin_id = atoi(argv[0]);
    lin_dev = lin_get_dev(lin_id);

    if (!lin_dev) {
        SDRV_LIN_DEMO_ERR(" lin id error\n");
        return -1;
    }

    transmit_pdu.pid = atoi(argv[1]);
    transmit_pdu.cs = LIN_ENHANCED_CS;
    transmit_pdu.drc = LIN_MASTER_RESPONSE;
    transmit_pdu.dl = 8;
    transmit_pdu.sdu = (uint8_t *)osAlloc(transmit_pdu.dl);
    if (transmit_pdu.sdu == NULL) {
        SDRV_LIN_DEMO_ERR(" malloc sdu fail\n");
        return -1;
    }

    receive_pdu.pid = atoi(argv[1]) + 1;
    receive_pdu.cs = LIN_ENHANCED_CS;
    receive_pdu.drc = LIN_SLAVE_RESPONSE;
    receive_pdu.dl = 8;
    receive_pdu.sdu = NULL;

    transfer_count = atoi(argv[2]);

    lin_wakeup(lin_dev);
    udelay(TIMEOUT_US);

    while (transfer_count) {
        ++summary_count;

        for (uint8_t i = 0; i < 8; ++i) {
            transmit_pdu.sdu[i] = transmit_pdu.pid + i;
        }

        if ((LIN_TX_OK != lin_get_status(lin_dev, &tx_sdu)) &&
            (LIN_RX_OK != lin_get_status(lin_dev, &rx_sdu))) {
            lin_abort(lin_dev);
        }

        lin_send_frame(lin_dev, &transmit_pdu);

        udelay(TIMEOUT_US);

        if (LIN_TX_OK != lin_get_status(lin_dev, &tx_sdu)) {
            lin_abort(lin_dev);
        }

        lin_send_frame(lin_dev, &receive_pdu);

        udelay(TIMEOUT_US);

        if (LIN_RX_OK != lin_get_status(lin_dev, &rx_sdu)) {
            lin_abort(lin_dev);
        }

        if (0 == strncmp((const char *)test_message, (const char *)rx_sdu, 8)) {
            ++lin_success;
            SDRV_LIN_DEMO_ALWAYS(" Sdrv lin master receive success! summary count "
                              "= %d, success count = %d, fail count = %d!\n",
                              summary_count, lin_success, lin_fail);
        } else {
            ++lin_fail;
            SDRV_LIN_DEMO_ALWAYS(" Sdrv lin master receive error!summary count = "
                              "%d, success count = %d, fail count = %d!\n",
                              summary_count, lin_success, lin_fail);
        }
        --transfer_count;
    }
    if (NULL != transmit_pdu.sdu) {
        osFree(transmit_pdu.sdu);
        transmit_pdu.sdu = NULL;
    }

    SDRV_LIN_DEMO_ALWAYS("\r\n Sdrv lin master stress test done!summary count = "
                       "%d, success count = %d, fail count = %d!\r\n",
                       summary_count, lin_success, lin_fail);
    return 0;
}

static int lin_get_status_demo(int argc, char *argv[])
{
    SDRV_LIN_DEMO_ALWAYS(" lin_get_status_demo start!\n");

    lin_dev_t *lin_dev;
    lin_status_t status;
    uint8_t lin_id;
    uint8_t *sdu;

    if (argc != 1) {
        SDRV_LIN_DEMO_ERR(" argc must be 1\n");
        return -1;
    }

    lin_id = atoi(argv[0]);
    lin_dev = lin_get_dev(lin_id);
    if (!lin_dev) {
        SDRV_LIN_DEMO_ERR(" lin id error\n");
        return -1;
    }

    status = lin_get_status(lin_dev, &sdu);

    SDRV_LIN_DEMO_ALWAYS(" get_status:%d\n", status);
    SDRV_LIN_DEMO_ALWAYS(" lin data:");
    for (uint8_t cnt = 0; cnt < LIN_FRAME_DATA_LENGTH - 1; cnt++) {
        SDRV_LIN_DEMO_ALWAYS("0x%x ", sdu[cnt]);
    }
    SDRV_LIN_DEMO_ALWAYS("\n");
    SDRV_LIN_DEMO_ALWAYS(" lin_get_status_demo success!\n");

    return 0;
}

int lin_deinit_demo(int argc, char *argv[])
{
    SDRV_LIN_DEMO_ALWAYS(" lin_deinit_demo start!\n");

    if (argc != 1) {
        SDRV_LIN_DEMO_ERR(" argc must be 1\n");
        return -1;
    }

    lin_dev_t *lin_dev;
    uint8_t lin_id;

    lin_id = atoi(argv[0]);
    lin_dev = lin_get_dev(lin_id);
    if (!lin_dev) {
        SDRV_LIN_DEMO_ERR(" lin id error\n");
        return -1;
    }

    lin_deinit(lin_dev);
    printf(" lin_deinit_demo success!\n");
    return 0;
}

static void lin_demo_usage(void)
{
    SDRV_LIN_DEMO_ALWAYS("lin_demo ls");
    SDRV_LIN_DEMO_ALWAYS("\t\t\t:lin dev list\n");

    SDRV_LIN_DEMO_ALWAYS("lin_demo wakeup_inter");
    SDRV_LIN_DEMO_ALWAYS("\t\t:lin dev wakeup internal [lin id]\n");
    SDRV_LIN_DEMO_ALWAYS(
        "\t\t\t\t must wakeup after lin init and before send frame\n");

    SDRV_LIN_DEMO_ALWAYS("lin_demo send");
    SDRV_LIN_DEMO_ALWAYS("\t\t\t:lin send frame demo [lin id] [pid]\n");

    SDRV_LIN_DEMO_ALWAYS("lin_demo recv");
    SDRV_LIN_DEMO_ALWAYS("\t\t\t:lin recv frame demo [lin id] [pid]\n");

    SDRV_LIN_DEMO_ALWAYS("lin_demo get_status");
    SDRV_LIN_DEMO_ALWAYS("\t\t:lin send gate status demo [lin id]\n");

    SDRV_LIN_DEMO_ALWAYS("lin_demo stress_test");
    SDRV_LIN_DEMO_ALWAYS("\t\t:lin stress test [lin id] [pid] [test count]\n");

    SDRV_LIN_DEMO_ALWAYS("lin_demo deinit");
    SDRV_LIN_DEMO_ALWAYS("\t\t\t:lin dev deinit.\n");
}

static int lin_demo_main(int argc, char *argv[])
{
    if (argc == 0) {
        lin_demo_usage();
        return 0;
    }

    if (!strcmp(argv[0], "ls")) {
        lin_dev_list(argc - 1, &argv[1]);
    } else if (!strcmp(argv[0], "wakeup_inter")) {
        lin_wakeup_internal_demo(argc - 1, &argv[1]);
    } else if (!strcmp(argv[0], "send")) {
        lin_send_frame_demo(argc - 1, &argv[1]);
    } else if (!strcmp(argv[0], "recv")) {
        lin_recv_frame_demo(argc - 1, &argv[1]);
    } else if (!strcmp(argv[0], "get_status")) {
        lin_get_status_demo(argc - 1, &argv[1]);
    } else if (!strcmp(argv[0], "stress_test")) {
        lin_stress_test(argc - 1, &argv[1]);
    } else if (!strcmp(argv[0], "deinit")) {
        lin_deinit_demo(argc - 1, &argv[1]);
    } else {
        SDRV_LIN_DEMO_ERR("error cmd\n");
    }
    return CLI_FALSE;
}

CLI_CMD("lin_demo", "\r\nlin_demo:\r\n lin device demo\n", lin_demo_main);