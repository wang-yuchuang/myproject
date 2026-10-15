/**
 * @file sdrv_uart_test.c
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
#include <driver/taishan/uart/sdrv_uart_drv.h>
#include <sdrv_uart.h>
#include <regs_base.h>
#include <driver/uart.h>

static char output_data[1024];
static char input_data[1024];
static uint32_t SDRV_UART_TEST;
static int32_t SDRV_UART_IRQ;

static sdrv_uart_t *uart_test_dev;
sdrv_uart_dev_t sdrv_test_uart;
uart_dev_t *test_uart;

static osSemaphoreId_t sem;

static uart_config_t test_uart_config = {
    .baud = 115200,
    .data_bits = UART_CHAR_8_BITS,
    .stop_bits = UART_STOP_1_BIT,
    .parity = UART_NO_PARITY,
    .clk_freq = 83000000,
};

static void uart_test_init(uint8_t uartNo)
{
    memset(output_data, 'z', 1024);
    memset(input_data, '\0', 1024);
    switch(uartNo){
        case 1:
            SDRV_UART_TEST = DEVICE_BASE(UART1);
            SDRV_UART_IRQ = DEVICE_INTR(UART1);
            break;
        case 2:
            SDRV_UART_TEST = DEVICE_BASE(UART2);
            SDRV_UART_IRQ = DEVICE_INTR(UART2);
            break;
        case 3:
            SDRV_UART_TEST = DEVICE_BASE(UART3);
            SDRV_UART_IRQ = DEVICE_INTR(UART3);
            break;
        case 4:
            SDRV_UART_TEST = DEVICE_BASE(UART4);
            SDRV_UART_IRQ = DEVICE_INTR(UART4);
            break;
        case 5:
            SDRV_UART_TEST = DEVICE_BASE(UART5);
            SDRV_UART_IRQ = DEVICE_INTR(UART5);
            break;
        case 6:
            SDRV_UART_TEST = DEVICE_BASE(UART6);
            SDRV_UART_IRQ = DEVICE_INTR(UART6);
            break;
        case 7:
            SDRV_UART_TEST = DEVICE_BASE(UART7);
            SDRV_UART_IRQ = DEVICE_INTR(UART7);
            break;
        case 8:
            SDRV_UART_TEST = DEVICE_BASE(UART8);
            SDRV_UART_IRQ = DEVICE_INTR(UART8);
            break;
        case 9:
            SDRV_UART_TEST = DEVICE_BASE(UART9);
            SDRV_UART_IRQ = DEVICE_INTR(UART9);
            break;
        case 10:
            SDRV_UART_TEST = DEVICE_BASE(UART10);
            SDRV_UART_IRQ = DEVICE_INTR(UART10);
            break;
        case 11:
            SDRV_UART_TEST = DEVICE_BASE(UART11);
            SDRV_UART_IRQ = DEVICE_INTR(UART11);
            break;
        case 12:
            SDRV_UART_TEST = DEVICE_BASE(UART12);
            SDRV_UART_IRQ = DEVICE_INTR(UART12);
            break;
        case 13:
            SDRV_UART_TEST = DEVICE_BASE(UART13);
            SDRV_UART_IRQ = DEVICE_INTR(UART13);
            break;
        case 14:
            SDRV_UART_TEST = DEVICE_BASE(UART14);
            SDRV_UART_IRQ = DEVICE_INTR(UART14);
            break;
        case 15:
            SDRV_UART_TEST = DEVICE_BASE(UART15);
            SDRV_UART_IRQ = DEVICE_INTR(UART15);
            break;
        case 16:
            SDRV_UART_TEST = DEVICE_BASE(UART16);
            SDRV_UART_IRQ = DEVICE_INTR(UART16);
            break;
        default:
            printf(" uart_test_init : Error!uart%d is invalid!\n", uartNo);
            SDRV_UART_TEST = DEVICE_BASE(UART16);
            SDRV_UART_IRQ = DEVICE_INTR(UART16);
    }
    uart_test_dev = (sdrv_uart_t *)SDRV_UART_TEST;
    if(uart_test_dev->MCR0.MODEN == 1)
    {
        printf(" uart_test_init : Warning!uart%d already init!\n", uartNo);
    }
    else
    {
        test_uart_config.base = SDRV_UART_TEST;
        test_uart_config.irq = SDRV_UART_IRQ;
        test_uart = sdrv_uart_probe(&sdrv_test_uart, &test_uart_config);
        uart_init(test_uart);
    }
}

static void uart_syncsend_test(int argc, char *argv[])
{
    printf("taishan uart test : uart_syncsend_test. \n");

    if (argc != 1) {
        printf("%s: argc num error:%d\n", __func__, argc);
        return;
    }
    uart_test_init(atoi(argv[0]));

    uart_transmit(test_uart, output_data, strlen(output_data));

}

static void uart_asyncsend_test_cb(void *arg)
{
    osSemaphoreRelease(sem);
    printf("uart_asyncsend_test_cb : uart asyncsend complete !\n");
}

static void uart_asyncsend_test(int argc, char *argv[])
{
    printf("taishan uart test : uart_asyncsend_test. \n");

    if (argc != 1) {
        printf("%s: argc num error:%d\n", __func__, argc);
        return;
    }

    uart_test_init(atoi(argv[0]));
    sem = osSemaphoreNew(1, 0, NULL);
    volatile uint8_t flag = 1;

    uart_transmit_async(test_uart, output_data, strlen(output_data), uart_asyncsend_test_cb);
    while(flag){
        if (osSemaphoreAcquire(sem, osWaitForever) == osOK) {
            printf("%s: success\n", __func__);
            flag = 0;
        }
    }
}

static void uart_syncrecv_test(int argc, char *argv[])
{
    printf("taishan uart test : uart_syncrecv_test test. \n");
    if (argc != 1) {
        printf("%s: argc num error:%d\n", __func__, argc);
        return;
    }

    uart_test_init(atoi(argv[0]));

    uart_receive(test_uart, input_data, 32);
    printf("uart recv data : %s", input_data);

}

static void uart_asyncrecv_test_cb(void *arg)
{
    osSemaphoreRelease(sem);
    printf("uart_asyncrecv_test_cb : uart asyncrecv complete !\n");
}

static void uart_asyncrecv_test(int argc, char *argv[])
{
    printf("taishan uart test : uart_asyncrecv_test. \n");
    if (argc != 1) {
        printf("%s: argc num error:%d\n", __func__, argc);
        return;
    }

    uart_test_init(atoi(argv[0]));
    sem = osSemaphoreNew(1, 0, NULL);
    volatile uint8_t flag = 1;

    uart_receive_async(test_uart, input_data, 32, uart_asyncrecv_test_cb);
    while(flag){
        if (osSemaphoreAcquire(sem, osWaitForever) == osOK) {
            printf("%s: success\n", __func__);
            flag = 0;
        }
    }
    printf("uart_asyncrecv_test : uart recv data : %s", input_data);

}

static void uart_receive_fovf_cb(void *arg)
{
    osSemaphoreRelease(sem);
    printf("uart_receive_fovf_cb : uart fifo overflow !\n");
}

static void uart_receive_fovf_test(int argc, char *argv[])
{
    printf("taishan uart test : uart_receive_fovf_test!\n");
    if (argc != 1) {
        printf("%s: argc num error:%d\n", __func__, argc);
        return;
    }

    volatile uint8_t flag = 1;
    sem = osSemaphoreNew(1, 0, NULL);
    uart_test_init(atoi(argv[0]));

    /* enable uart rx fifo overflow intr */
    uart_receive_fovf_enable(test_uart, true, uart_receive_fovf_cb);

    printf("uart_receive_fovf_test! Pls send data more than 32 bytes by PC!\n");
    while(flag){
        if (osSemaphoreAcquire(sem, osWaitForever) == osOK) {
            printf("%s: success\n", __func__);
            flag = 0;
        }
    }
}

static void uart_test_usage(void)
{
    printf("\n");
    printf("Pls connect the dev with PC!\n");
    printf("uart_test syncsend [uartNo]");
    printf("\t\t\t:uart syncsend test\n");

    printf("uart_test asyncsend [uartNo]");
    printf("\t\t:uart asyncsend test\n");

    printf("uart_test syncrecv [uartNo]");
    printf("\t\t:uart syncrecv test\n");

    printf("uart_test asyncrecv [uartNo]");
    printf("\t\t:uart asyncrecv test\n");

    printf("uart_test recvfovf [uartNo]");
    printf("\t\t:uart recvfovf test\n");
}

static int uart_test(int argc, char *argv[])
{
    if (argc == 0) {
        uart_test_usage();
        return CLI_FALSE;
    }

    else if (!strcmp(argv[0], "syncsend")) {
        uart_syncsend_test(argc - 1, &argv[1]);
    }
    else if (!strcmp(argv[0], "asyncsend")) {
        uart_asyncsend_test(argc - 1, &argv[1]);
    }
    else if (!strcmp(argv[0], "syncrecv")) {
        uart_syncrecv_test(argc - 1, &argv[1]);
    }
    else if (!strcmp(argv[0], "asyncrecv")) {
        uart_asyncrecv_test(argc - 1, &argv[1]);
    }
    else if (!strcmp(argv[0], "recvfovf")){
        uart_receive_fovf_test(argc - 1, &argv[1]);
    }
    else {
        printf("cmd error:%s\n", argv[0]);
    }

    return CLI_FALSE;
}

CLI_CMD("uart_test", "\r\nuart_test:\r\n uart test\r\n", uart_test);
