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
#include <driver/kunlun/uart/dw_uart_drv.h>
#include <regs_base.h>
#include <driver/uart.h>

#define UART_SAF_CLK        80000000
#define UART_SEC0_CLK       72000000
#define UART_SEC1_CLK       72000000

static char output_data[1024];
static char input_data[1024];
static uint32_t SDRV_UART_TEST;
static int32_t SDRV_UART_IRQ;
static int32_t SDRV_UART_CLK_FREQ;

static dw_uart_dev_t *uart_test_dev;
dw_uart_dev_t sdrv_test_uart;
uart_dev_t *test_uart;

static osSemaphoreId_t sem;

static uart_config_t test_uart_config = {
    .baud = 115200,
    .data_bits = UART_CHAR_8_BITS,
    .stop_bits = UART_STOP_1_BIT,
    .parity = UART_NO_PARITY,
    .clk_freq = 120000000,
};
const char uart_message[64] = "12345678123456781234567812345678";

static void uart_test_init(uint8_t uartNo)
{
    memset(output_data, 'z', 1024);
    memset(input_data, '\0', 1024);
    switch(uartNo){
        case 1:
            SDRV_UART_TEST = DEVICE_BASE(UART1);
            SDRV_UART_IRQ = DEVICE_INTR(UART1);
            SDRV_UART_CLK_FREQ = UART_SAF_CLK;
            break;
        case 2:
            SDRV_UART_TEST = DEVICE_BASE(UART2);
            SDRV_UART_IRQ = DEVICE_INTR(UART2);
            SDRV_UART_CLK_FREQ = UART_SAF_CLK;
            break;
        case 3:
            SDRV_UART_TEST = DEVICE_BASE(UART3);
            SDRV_UART_IRQ = DEVICE_INTR(UART3);
            SDRV_UART_CLK_FREQ = UART_SAF_CLK;
            break;
        case 4:
            SDRV_UART_TEST = DEVICE_BASE(UART4);
            SDRV_UART_IRQ = DEVICE_INTR(UART4);
            SDRV_UART_CLK_FREQ = UART_SAF_CLK;
            break;
        case 5:
            SDRV_UART_TEST = DEVICE_BASE(UART5);
            SDRV_UART_IRQ = DEVICE_INTR(UART5);
            SDRV_UART_CLK_FREQ = UART_SAF_CLK;
            break;
        case 6:
            SDRV_UART_TEST = DEVICE_BASE(UART6);
            SDRV_UART_IRQ = DEVICE_INTR(UART6);
            SDRV_UART_CLK_FREQ = UART_SAF_CLK;
            break;
        case 7:
            SDRV_UART_TEST = DEVICE_BASE(UART7);
            SDRV_UART_IRQ = DEVICE_INTR(UART7);
            SDRV_UART_CLK_FREQ = UART_SAF_CLK;
            break;
        case 8:
            SDRV_UART_TEST = DEVICE_BASE(UART8);
            SDRV_UART_IRQ = DEVICE_INTR(UART8);
            SDRV_UART_CLK_FREQ = UART_SAF_CLK;
            break;
        case 9:
            SDRV_UART_TEST = DEVICE_BASE(UART9);
            SDRV_UART_IRQ = DEVICE_INTR(UART9);
            SDRV_UART_CLK_FREQ = UART_SEC0_CLK;
            break;
        case 10:
            SDRV_UART_TEST = DEVICE_BASE(UART10);
            SDRV_UART_IRQ = DEVICE_INTR(UART10);
            SDRV_UART_CLK_FREQ = UART_SEC1_CLK;
            break;
        case 11:
            SDRV_UART_TEST = DEVICE_BASE(UART11);
            SDRV_UART_IRQ = DEVICE_INTR(UART11);
            SDRV_UART_CLK_FREQ = UART_SEC0_CLK;
            break;
        case 12:
            SDRV_UART_TEST = DEVICE_BASE(UART12);
            SDRV_UART_IRQ = DEVICE_INTR(UART12);
            SDRV_UART_CLK_FREQ = UART_SEC1_CLK;
            break;
        case 13:
            SDRV_UART_TEST = DEVICE_BASE(UART13);
            SDRV_UART_IRQ = DEVICE_INTR(UART13);
            SDRV_UART_CLK_FREQ = UART_SEC0_CLK;
            break;
        case 14:
            SDRV_UART_TEST = DEVICE_BASE(UART14);
            SDRV_UART_IRQ = DEVICE_INTR(UART14);
            SDRV_UART_CLK_FREQ = UART_SEC1_CLK;
            break;
        case 15:
            SDRV_UART_TEST = DEVICE_BASE(UART15);
            SDRV_UART_IRQ = DEVICE_INTR(UART15);
            SDRV_UART_CLK_FREQ = UART_SEC0_CLK;
            break;
        case 16:
            SDRV_UART_TEST = DEVICE_BASE(UART16);
            SDRV_UART_IRQ = DEVICE_INTR(UART16);
            SDRV_UART_CLK_FREQ = UART_SEC1_CLK;
            break;

        default:
            printf(" uart_test_init : Error!uart%d is invalid!\n", uartNo);
            SDRV_UART_TEST = DEVICE_BASE(UART1);
            SDRV_UART_IRQ = DEVICE_INTR(UART1);
    }
    uart_test_dev = (dw_uart_dev_t *)SDRV_UART_TEST;
    test_uart_config.base = SDRV_UART_TEST;
    test_uart_config.irq = SDRV_UART_IRQ;
    test_uart_config.clk_freq = SDRV_UART_CLK_FREQ;
    test_uart = dw_uart_probe(&sdrv_test_uart, &test_uart_config);
    uart_init(test_uart);
}

static void uart_syncsend_test(int argc, char *argv[])
{
    printf("kunlun uart test : uart_syncsend_test. \n");

    if (argc != 1) {
        printf("%s: argc num error:%d\n", __func__, argc);
        return;
    }
    uart_test_init(atoi(argv[0]));

    test_uart->ops->transmit(test_uart, output_data, strlen(output_data));
    test_uart->ops->transmit(test_uart, "\r\n", 2U);
    printf("%s: success!\r\n", __func__);

}

static void uart_asyncsend_test_cb(void *arg)
{
    osSemaphoreRelease(sem);
    printf("uart_asyncsend_test_cb : uart asyncsend complete !\n");
}

static void uart_asyncsend_test(int argc, char *argv[])
{
    printf("kunlun uart test : uart_asyncsend_test. \n");

    if (argc != 1) {
        printf("%s: argc num error:%d\n", __func__, argc);
        return;
    }

    uart_test_init(atoi(argv[0]));
    sem = osSemaphoreNew(1, 0, NULL);
    volatile uint8_t flag = 1;

    test_uart->ops->transmit_async(test_uart, output_data, strlen(output_data));
    while(flag){
        if (osSemaphoreAcquire(sem, osWaitForever) == osOK) {
            printf("%s: success!\r\n", __func__);
            flag = 0;
        }
    }
}

static void uart_syncrecv_test(int argc, char *argv[])
{
    printf("kunlun uart test : uart_syncrecv_test test. \n");
    printf("Pls input uart_message. \n");
    if (argc != 1) {
        printf("%s: argc num error:%d\r\n", __func__, argc);
        return;
    }

    uart_test_init(atoi(argv[0]));

    uart_receive(test_uart, input_data, 32);
    printf("uart recv data : %s\r\n", input_data);
    if(strncmp(input_data, uart_message, 32))
    {
        printf("%s: fail!\r\n", __func__);
    }else{
        printf("%s: success!\r\n", __func__);
    }
}

static void uart_asyncrecv_test_cb(void *arg)
{
    osSemaphoreRelease(sem);
    printf("uart_asyncrecv_test_cb : uart asyncrecv complete !\n");
}

static void uart_asyncrecv_test(int argc, char *argv[])
{
    printf("kunlun uart test : uart_asyncrecv_test. \n");
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
            printf("uart_asyncrecv_test : uart recv data : %s\r\n", input_data);
            if(strncmp(input_data, uart_message, 32))
            {
                printf("%s: fail!\r\n", __func__);
            }else{
                printf("%s: success!\r\n", __func__);
            }
            flag = 0;
        }
    }
}

static void uart_receive_fovf_cb(void *arg)
{
    printf("uart_receive_fovf_cb : uart fifo overflow !\r\n");
    osSemaphoreRelease(sem);
}

static void uart_receive_fovf_test(int argc, char *argv[])
{
    printf("kunlun uart test : uart_receive_fovf_test!\r\n");
    if (argc != 1) {
        printf("%s: argc num error:%d\n", __func__, argc);
        return;
    }

    volatile uint8_t flag = 1;
    sem = osSemaphoreNew(1, 0, NULL);
    uart_test_init(atoi(argv[0]));

    /* enable uart rx fifo overflow intr */
    uart_receive_fovf_enable(test_uart, true, uart_receive_fovf_cb);
    printf("Pls send data more than 32 bytes by PC!\r\n");
    while(flag){
        if (osSemaphoreAcquire(sem, osWaitForever) == osOK) {
            printf("%s: success\r\n", __func__);
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

CLI_CMD("uart_test", "uart_test: send 12345678123456781234567812345678 or receiv data.\n\r", uart_test);
