/**
 * @file demo_can.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: canfd demo for taishan.
 *
 * Revision History:
 * -----------------
 */

#include <debug.h>
#include <stdlib.h>
#include <string.h>

#include "CLI.h"
#include "can_cfg.h"
#include "driver/taishan/can/flexcan_driver.h"
#include "driver/can.h"
#include "irq_num.h"
#include "regs_base.h"

static void Can_Error_Notify(can_device_t *can_dev, enum can_errcode code)
{
    ASSERT(can_dev);

    switch (code) {
        case CAN_BUSOFF:
            ssdk_printf(SSDK_INFO, "can_busoff\n");
            break;
        case CAN_WAKEUP:
            can_request_sleep(can_dev, false);
            ssdk_printf(SSDK_INFO, "CAN_WAKEUP\n");
            break;
        case CAN_NO_ACK:
            ssdk_printf(SSDK_INFO, "CAN_NO_ACK\n");
            break;
        case CAN_BIT_ERROR:
            ssdk_printf(SSDK_INFO, "CAN_BIT_ERROR\n");
            break;
        case CAN_RX_OVERFLOW:
            ssdk_printf(SSDK_INFO, "can_rx_overflow\n");
            break;
        default:
            break;
    }
}

static void can_tx_thread(void *arg)
{
    can_device_t *can_dev = (can_device_t *)arg;

    while (true) {
        can_transmit_buf(can_dev);
    }
}

static void can_rx_thread(void *arg)
{
    can_device_t *can_dev = (can_device_t *)arg;
    struct can_frame frame;
    int ret;

    while (true) {
        ret = can_receive_buf(can_dev, &frame);

        if (!ret) {
            printf("id=0x%x len=%d\n", frame.id, frame.len);
            for (int i = 0; i < frame.len; i++) {
                printf("data[%d]=0x%x ", i, frame.data[i]);
            }
            printf("\n");
        }
    }
}

static void can_create_tx_thread(can_device_t *can_dev)
{
    char tx_name[20];
    osThreadId_t tx_thread;

    sprintf(tx_name, "can_tx_thread%d", can_dev->ch);
    osThreadAttr_t tx_attr = {.name = tx_name, .priority = osPriorityNormal};
    tx_thread = osThreadNew(can_tx_thread, can_dev, &tx_attr);
    ASSERT(tx_thread);
}

static void can_create_rx_thread(can_device_t *can_dev)
{
    char rx_name[20];
    osThreadId_t rx_thread;

    sprintf(rx_name, "can_rx_thread%d", can_dev->ch);
    osThreadAttr_t rx_attr = {.name = rx_name, .priority = osPriorityNormal};
    rx_thread = osThreadNew(can_rx_thread, can_dev, &rx_attr);
    ASSERT(rx_thread);
}

static int can_init_demo(int argc, char *argv[])
{
    if (argc != 1) {
        ssdk_printf(SSDK_INFO, "\nargc must be 1");
        ssdk_printf(SSDK_INFO, "\t\t:can_demo init [can_id] \r\n");
        return 0;
    }
    int can_id = 0;

    can_id = strtoul(argv[0], NULL, 0);

    can_device_t *can_dev = can_get_probe_device(can_id);

    if (can_dev == NULL) {
        printf("id = %d CAN Controller not Support \r\nuse can_demo ls command to know which can id can be used\r\n", can_id);
        return 0;
    }

    can_create_tx_thread(can_dev);
    can_create_rx_thread(can_dev);

    can_set_err_notify(can_dev, Can_Error_Notify);

    return 0;
}

static int can_device_list(int argc, char *argv[])
{
    can_device_t *can_dev = NULL;
    int can_hw_id = 0;
    int can_hw_id_max = 32;

    for (can_hw_id = 0; can_hw_id < can_hw_id_max; can_hw_id++) {
        can_dev = can_get_probe_device(can_hw_id);
        if (can_dev != NULL) {
            printf("can %d is availabe\r\n",can_hw_id);
        }
    }
    return 0;
}
static int can_write_data(int argc, char *argv[])
{
    uint8_t Data[64] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
                        13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                        26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
                        39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
                        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63};

    struct can_frame frame;
    int count, mb_idx;
    int ret = 0;
    int can_id = 0;

    can_id = strtoul(argv[0], NULL, 0);

    can_device_t *can_dev = can_get_probe_device(can_id);

    if (can_dev == 0) {
        printf("id = %d CAN Controller not Support \r\nuse can_demo ls command to know which can id can be used\r\n", can_id);
        return 0;
    }

    if (argc != 5) {
        ssdk_printf(SSDK_INFO, "\nargc must be 5");
        ssdk_printf(SSDK_INFO,
                    "\t\t:can_demo write [can_controller_index] [mb_idx] [id] [fd] [count]\n");
        return 0;
    }

    mb_idx = strtoul(argv[1], NULL, 0);
    frame.id = strtoul(argv[2], NULL, 0);
    frame.fd = strtoul(argv[3], NULL, 0);

    if (frame.fd)
        frame.len = 64;
    else
        frame.len = 8;

    frame.ide = CAN_STDID;
    frame.rtr = CAN_DTR;
    memcpy(&frame.data, &Data, frame.len);

    count = strtoul(argv[4], NULL, 0);

    for (int i = 0; i < count; i++) {
        ret = can_write(can_dev, &frame, mb_idx);
        osDelay(1);
        if (ret) {
            ssdk_printf(SSDK_INFO, "can write failed\n");
        }
    }

    return 0;
}

static int can_sleep_test(int argc, char *argv[])
{
    if (argc != 2) {
        ssdk_printf(SSDK_INFO, "\nargc must be 2");
        ssdk_printf(SSDK_INFO, "\t\t:can_demo request_sleep [can_id] [request] \r\n");
        return 0;
    }

    int can_id = 0;
    can_id = strtoul(argv[0], NULL, 0);

    can_device_t *can_dev = can_get_probe_device(can_id);

    if (can_dev == NULL) {
        printf("id = %d CAN Controller not Support \r\nuse can_demo ls command to know which can id can be used\r\n", can_id);
        return 0;
    }

    bool request = strtoul(argv[1], NULL, 0);

    can_request_sleep(can_dev, request);

    return 0;
}

static void can_demo_usage(void)
{
    ssdk_printf(SSDK_CRIT, "can_demo ls");
    ssdk_printf(SSDK_CRIT, "\t\t\t:list all available can device\n");

    ssdk_printf(SSDK_CRIT, "can_demo init");
    ssdk_printf(SSDK_CRIT, "\t\t\t:can_demo init [can_id]\n");

    ssdk_printf(SSDK_CRIT, "can_demo write");
    ssdk_printf(SSDK_CRIT,
                "\t\t\t:can_demo write [can_id][mb_idx] [id] [fd] [count]\n");

    ssdk_printf(SSDK_CRIT, "can_demo request_sleep");
    ssdk_printf(SSDK_CRIT, "\t\t:can_demo request_sleep [can_id] [request]\n");
}

static int can_demo_main(int argc, char *argv[])
{
    if (argc == 0) {
        can_demo_usage();
        return 0;
    }

    if (!strcmp(argv[0], "init")) {
        can_init_demo(argc - 1, &argv[1]);
    } else if (!strcmp(argv[0], "write")) {
        can_write_data(argc - 1, &argv[1]);
    } else if (!strcmp(argv[0], "ls")) {
        can_device_list(argc - 1, &argv[1]);
    } else if (!strcmp(argv[0], "request_sleep")) {
        can_sleep_test(argc - 1, &argv[1]);
    } else {
        ssdk_printf(SSDK_CRIT, "error cmd\n");
    }

    return CLI_FALSE;
}

CLI_CMD("can_demo", "\r\ncan_demo:\r\n can device demo\n", can_demo_main);
