/**
 * @file can_test.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: add board test For CANFD.
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include <string.h>
#include <debug.h>

#include "CLI.h"
#include "regs_base.h"
#include "irq_num.h"
#include "driver/taishan/can/flexcan_driver.h"
#include "can_cfg.h"

static void Can_Error_Notify(can_device_t *can_dev, enum can_errcode code)
{
    ASSERT(can_dev);

    switch (code) {
        case CAN_BUSOFF:
            ssdk_printf(SSDK_INFO, "can_busoff\n");
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

static int can_test_device_list(int argc, char *argv[])
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

static int can_test_write(int argc, char *argv[])
{
    uint8_t Data[64] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
                        13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                        26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
                        39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
                        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63};

    struct can_frame frame;
    int can_id, mb_idx;
    int ret = 0;
    can_device_t *can_dev;

    if (argc != 4) {
        ssdk_printf(SSDK_INFO, "\nargc must be 4");
        ssdk_printf(SSDK_INFO, "\t\t:can_test write [can_id] [mb_idx] [id] [fd]\n");
        return 0;
    }

    can_id = strtoul(argv[0], NULL, 0);
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

    can_dev = can_get_probe_device(can_id);
    if (can_dev == NULL) {
        printf("id = %d CAN Controller not Support \r\nuse can_test ls command to know which can id can be used\r\n", can_id);
        return 0;
    }

    ret = can_write(can_dev, &frame, mb_idx);
    if (ret) {
        ssdk_printf(SSDK_INFO, "can write failed\n");
    }
    can_transmit_buf(can_dev);

    return 0;
}

static int can_test_read(int argc, char *argv[])
{
    int can_id;
    can_device_t *can_dev;
    struct can_frame frame;
    int ret = 0;

    if(argc != 1) {
        ssdk_printf(SSDK_INFO, "\nargc must be 1");
        return 0;
    }

    can_id = strtoul(argv[0], NULL, 0);
    can_dev = can_get_probe_device(can_id);
    if (can_dev == NULL) {
        printf("id = %d CAN Controller not Support \r\nuse can_test ls command to know which can id can be used\r\n", can_id);
        return 0;
    }

    ret = can_receive_buf(can_dev, &frame);
    if (!ret) {
        printf("id=0x%x len=%d\n", frame.id, frame.len);
        for (int i = 0; i < frame.len; i++) {
            printf("data[%d]=0x%x ", i, frame.data[i]);
        }
        printf("\n");
    }

    return 0;
}

static int can_test_set_notify(int argc, char *argv[])
{
    int can_id;
    can_device_t *can_dev;

    if(argc != 1) {
        ssdk_printf(SSDK_INFO, "\nargc must be 1");
        return 0;
    }

    can_id = strtoul(argv[0], NULL, 0);
    can_dev = can_get_probe_device(can_id);
    if (can_dev == NULL) {
        printf("id = %d CAN Controller not Support \r\nuse can_test ls command to know which can id can be used\r\n", can_id);
        return 0;
    }

    can_set_err_notify(can_dev, Can_Error_Notify);

    return 0;
}

static int can_test_request_sleep(int argc, char *argv[])
{
    int can_id;
    can_device_t *can_dev;
    bool request;

    if(argc != 2) {
        ssdk_printf(SSDK_INFO, "\nargc must be 2");
        return 0;
    }

    can_id = strtoul(argv[0], NULL, 0);
    request = strtoul(argv[1], NULL, 0);
    can_dev = can_get_probe_device(can_id);
    if (can_dev == NULL) {
        printf("id = %d CAN Controller not Support \r\nuse can_test ls command to know which can id can be used\r\n", can_id);
        return 0;
    }

    can_request_sleep(can_dev, request);

    return 0;
}

static void can_test_usage(void)
{
    ssdk_printf(SSDK_CRIT, "can_test ls");
    ssdk_printf(SSDK_CRIT, "\t\t:list all available can device\n");

    ssdk_printf(SSDK_CRIT, "can_test write");
    ssdk_printf(SSDK_CRIT, "\t\t:can_test write [can_id] [mb_idx] [id] [fd]\n");

    ssdk_printf(SSDK_CRIT, "can_test read");
    ssdk_printf(SSDK_CRIT, "\t\t:can_test read [can_id]\n");

    ssdk_printf(SSDK_CRIT, "can_test set_notify");
    ssdk_printf(SSDK_CRIT, "\t:can_test set_notify [can_id]\n");

    ssdk_printf(SSDK_CRIT, "can_test request_sleep");
    ssdk_printf(SSDK_CRIT, "\t:can_test request_sleep [can_id] [request]\n");
}

static int can_test_main(int argc, char *argv[])
{
    if (argc == 0) {
        can_test_usage();
        return 0;
    }

    if (!strcmp(argv[0], "read")) {
        can_test_read(argc - 1, &argv[1]);
    }
    else if (!strcmp(argv[0], "write")) {
        can_test_write(argc - 1, &argv[1]);
    }
    else if (!strcmp(argv[0], "set_notify")) {
        can_test_set_notify(argc - 1, &argv[1]);
    }
    else if (!strcmp(argv[0], "request_sleep")) {
        can_test_request_sleep(argc - 1, &argv[1]);
    }
    else if (!strcmp(argv[0], "ls")) {
        can_test_device_list(argc - 1, &argv[1]);
    }
    else {
        ssdk_printf(SSDK_CRIT, "error cmd\n");
    }

    return CLI_FALSE;
}

CLI_CMD("can_test", "\r\ncan_test:\r\n add board test for canfd\n", can_test_main);
