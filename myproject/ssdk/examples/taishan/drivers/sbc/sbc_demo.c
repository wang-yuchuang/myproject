/*
 * sbc_demo.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Board level SBC example.
 *
 * Revision History:
 * -----------------
 */

#include <board.h>
#include <clock_cfg.h>
#include <cmsis_os2.h>
#include <driver/pinctrl.h>
#include <driver/taishan/pinctrl/taishan_pinctrl.h>
#include <pinmux_cfg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CLI_type.h"
#include "TLE94x1.h"
#include "sbc_sys.h"
#include "driver/taishan/can/flexcan_driver.h"
#include "driver/gpio.h"
#include "driver/pinctrl.h"

extern gpio_dev_t g_gpio_safety;
extern gpio_dev_t g_gpio_ap;
#define GPIO_INSTANCE(pin)                                                     \
    (((pin) < TAISHAN_SAFETY_PIN_NUM) ? &g_gpio_safety : &g_gpio_ap)

extern pinctrl_t g_pinctrl_safety;
extern pinctrl_t g_pinctrl_ap;
#define PINCTRL_INSTANCE(pin)                                                  \
    (((pin) < TAISHAN_SAFETY_PIN_NUM) ? &g_pinctrl_safety : &g_pinctrl_ap)

#define INTN_PIN GPIO_S8

static int sbc_main(int argc, char *argv[])
{
    int i = 0;
    uint8_t addr = 0, data = 0;
    SBC_ErrorCode error = {0};
    uint16_t rltvalue = 0;

    for (i = 0; i < argc; i++) {
        if (argv[i][0] != '-')
            continue;

        switch (argv[i][1]) {
        case 'r':
            i++;
            if (i < argc) {
                addr = atoi(argv[i]);
            }
            printf("TLE9461 Read Success \r\nAddress[0x%x] = 0x%x", addr,
                   SBC_Read_Command(addr));

            break;

        case 'w':
            i++;
            if (i < argc) {
                addr = atoi(argv[i]);
            }
            i++;
            if (i < argc) {
                data = atoi(argv[i]);
            }
            error = SBC_Write_Reg(addr, data, &rltvalue);
            printf("SBC_Register = 0x%x\r\n", error.SBC_Register);
            printf("flippedBitsMask= 0x%x\r\n", error.flippedBitsMask);
            printf("expectedValue= 0x%x\r\n", error.expectedValue);
            break;

        case 'h':
            printf("read sbc register Usage:sbc -r address\r\n");
            printf("write sbc register Usage:sbc -w address data\r\n");
            printf("read sbc status Usage:sbc -s\r\n");
            printf("sbc debug function Usage:sbc -d DEBUG_CMD\r\n");
            break;

        case 'd':
            i++;
            /*set FO as GPIO and output low vol for 1000 ms */
            if (!strcmp(argv[i], "FO_TEST")) {
                SBC_FO_Test_On();
                osDelay(1000);
                SBC_FO_Test_Off();
            }
            /*Enter Sleep Mode First of lp */
            if (!strcmp(argv[i], "SBC_STOP")) {
                SBC_Mode_Stop();
                printf("\nSBC Enter Stop Mode\r\n");
            }
            if (!strcmp(argv[i], "SBC_SLEEP")) {
                SBC_Unlock_Configuration();
                error = SBC_Mode_Sleep();
                SBC_Lock_Configuration();
                if (error.flippedBitsMask == 0) {
                    printf("\nSBC Enter Sleep Mode\r\n");
                } else {
                    printf(
                        "\nerror.SBC_Register = 0x%x\r\nerror.flippedBitsMask "
                        "= 0x%x\r\nerror.expectedValue = 0x%x\r\n",
                        error.SBC_Register, error.flippedBitsMask,
                        error.expectedValue);
                }
            }
            if (!strcmp(argv[i], "SBC_NORMAL")) {
                SBC_Mode_Normal();
                printf("\nSBC Enter Normal Mode\r\n");
            }
            if (!strcmp(argv[i], "SBC_RESET")) {
                SBC_Soft_Reset();
                printf("\nSBC RESET\r\n");
            }
            if (!strcmp(argv[i], "FEED_WTD")) {
                SBC_WD_Trigger();
                printf("\nTrigger wtd\r\n");
            }
            if (!strcmp(argv[i], "STOP_WTD_SERV")) {
                SBC_Suspend_WD_Trigger_Task();
                printf("\nstop wtd Trigger server\r\n");
            }
            if (!strcmp(argv[i], "SET_PERIOD")) {
                i++;
                if (i < argc) {
                    rltvalue = atoi(argv[i]);
                }
                SBC_Set_WD_Trigger_Period(rltvalue);
                printf("\nset Trigger period = %d ms success\r\n", rltvalue);
            }
            if (!strcmp(argv[i], "GET_PERIOD")) {
                printf("Trigger period = %d ms \r\n",
                       SBC_Get_WD_Trigger_Period());
            }
            if (!strcmp(argv[i], "SET_DUTYCYCLE_PWM")) {
                i++;
                if (i < argc) {
                    rltvalue = atoi(argv[i]);
                }
                SBC_Set_DutyCycle_PWM(rltvalue);
                printf("\nset Duty Cycle PWM = %d success\r\n", rltvalue);
            }
            if (!strcmp(argv[i], "SET_TIMER_ON")) {
                i++;
                if (i < argc) {
                    rltvalue = atoi(argv[i]);
                }
                SBC_Set_Timer_On(rltvalue);
                printf("\nset Timer On = %d success\r\n", rltvalue);
            }
            if (!strcmp(argv[i], "SET_TIMER_PERIOD")) {
                i++;
                if (i < argc) {
                    rltvalue = atoi(argv[i]);
                }
                SBC_Set_Timer_Period(rltvalue);
                printf("\nset Timer Period = %d success\r\n", rltvalue);
            }
            if (!strcmp(argv[i], "CP_ON")) {
                SBC_CP_On();
                printf("\ncp on success\r\n");
            }
            if (!strcmp(argv[i], "CP_OFF")) {
                SBC_CP_Off();
                printf("\ncp off success\r\n");
            }
            if (!strcmp(argv[i], "MEAS_ON")) {
                SBC_WK_MEAS_On();
                printf("\nmeasure on success\r\n");
            }
            if (!strcmp(argv[i], "MEAS_OFF")) {
                SBC_WK_MEAS_Off();
                printf("\nmeasure off success\r\n");
            }
            if (!strcmp(argv[i], "WK_ON")) {
                SBC_TIMER_WK_EN_On();
                printf("\nwk on success\r\n");
            }
            if (!strcmp(argv[i], "WK_OFF")) {
                SBC_TIMER_WK_EN_Off();
                printf("\nwk off success\r\n");
            }
            if (!strcmp(argv[i], "CONF_UNLOCK")) {
                SBC_Unlock_Configuration();
                printf("\nlock register success\r\n");
            }
            if (!strcmp(argv[i], "CONF_LOCK")) {
                SBC_Lock_Configuration();
                printf("\nunlock register success\r\n");
            }
            break;
        case 'v':
            printf("Compile Time is %s %s", __DATE__, __TIME__);
            break;
        default:
            printf("\nunknown options: %s\n", argv[i]);
            break;
        }
    }

    return 0;
}

static void irqSBC_Handler(struct gpio_dev *dev, int pin, void *arg)
{
    SBC_ISR();
}

static void irqSBC_Wake_Handler(uint8_t value)
{
    printf("event on WK input occured . Value = 0x%x\r\n", value);
}

static void irqSBC_CAN_Wake_Handler(uint8_t value)
{
    printf("event on CAN occured . Value = 0x%x\r\n", value);
}

static void irqSBC_Timer_Wake_Handler(uint8_t value)
{
    printf("event on Timer occured . Value = 0x%x\r\n", value);
}

static void irqSBC_GPIO_Wake_Handler(uint8_t value)
{
    printf("event on GPIO input occured . Value = 0x%x\r\n", value);
}

static void irqSBC_ISR_VS_UV_Handler(uint8_t value)
{
    printf("event on SBC_ISR_VS_UV occured . Value = 0x%x\r\n", value);
}
static void irqSBC_ISR_VS_OV_Handler(uint8_t value)
{
    printf("event on SBC_ISR_VS_OV occured . Value = 0x%x\r\n", value);
}
static void irqSBC_ISR_VCC1_OV_Handler(uint8_t value)
{
    printf("event on SBC_ISR_VCC1_OV occured . Value = 0x%x\r\n", value);
}
static void irqSBC_ISR_VCC1_UV_PREWARN_Handler(uint8_t value)
{
    printf("event on SBC_ISR_VCC1_UV_PREWARN occured . Value = 0x%x\r\n", value);
}
static void irqSBC_ISR_VCC2_OT_Handler(uint8_t value)
{
    printf("event on SBC_ISR_VCC2_OT occured . Value = 0x%x\r\n", value);
}
static void irqSBC_ISR_VCC2_UV_Handler(uint8_t value)
{
    printf("event on SBC_ISR_VCC2_UV occured . Value = 0x%x\r\n", value);
}
static void irqSBC_ISR_VCC1_UV_Handler(uint8_t value)
{
    printf("event on SBC_ISR_VCC1_UV occured . Value = 0x%x\r\n", value);
}
static void irqSBC_ISR_TSD2_SAFESTATE_Handler(uint8_t value)
{
    printf("event on SBC_ISR_TSD2_SAFESTATE occured . Value = 0x%x\r\n", value);
}
static void irqSBC_ISR_TSD1_Handler(uint8_t value)
{
    printf("event on SBC_ISR_TSD1 occured . Value = 0x%x\r\n", value);
}
static void irqSBC_ISR_TPW_Handler(uint8_t value)
{
    printf("event on SBC_ISR_TPW occured . Value = 0x%x\r\n", value);
}
static void irqSBC_ISR_RESTART_AFTER_FAIL_Handler(uint8_t value)
{
    printf("event on SBC_ISR_RESTART_AFTER_FAIL occured . Value = 0x%x\r\n", value);
}
static void irqSBC_ISR_FROM_SLEEPMODE_Handler(uint8_t value)
{
    printf("event on SBC_ISR_FROM_SLEEPMODE occured . Value = 0x%x\r\n", value);
}
static void irqSBC_ISR_WD_FAIL_1_Handler(uint8_t value)
{
    printf("event on SBC_ISR_WD_FAIL_1  occured . Value = 0x%x\r\n", value);
}
static void irqSBC_ISR_WD_FAIL_2_Handler(uint8_t value)
{
    printf("event on SBC_ISR_WD_FAIL_2 occured . Value = 0x%x\r\n", value);
}
static void irqSBC_ISR_FAILURE_Handler(uint8_t value)
{
    printf("event on SBC_ISR_FAILURE occured . Value = 0x%x\r\n", value);
}
static void irqSBC_ISR_CAN_TSD_Handler(uint8_t value)
{
    printf("event on SBC_ISR_CAN_TSD occured . Value = 0x%x\r\n", value);
}
static void irqSBC_ISR_CAN_TXD_DOM_TO_Handler(uint8_t value)
{
    printf("event on SBC_ISR_CAN_TXD_DOM_TO  occured . Value = 0x%x\r\n", value);
}
static void irqSBC_ISR_CAN_BUS_DOM_TO_Handler(uint8_t value)
{
    printf("event on SBC_ISR_CAN_BUS_DOM_TO occured . Value = 0x%x\r\n", value);
}
static void irqSBC_ISR_VCAN_UV_Handler(uint8_t value)
{
    printf("event on SBC_ISR_VCAN_UV occured . Value = 0x%x\r\n", value);
}
static void irqSBC_ISR_GPIO_OC_Handler(uint8_t value)
{
    printf("event on SBC_ISR_GPIO_OC occured . Value = 0x%x\r\n", value);
}
static void irqSBC_ISR_GPIO_OL_Handler(uint8_t value)
{
    printf("event on SBC_ISR_GPIO_OL occured . Value = 0x%x\r\n", value);
}

static int sbc_monitor_serve_start_main(int argc, char *argv[])
{
    gpio_dev_t *gpio_dev = GPIO_INSTANCE(INTN_PIN);

    /* handle interrupts from SBC by rising edge on INTN */
    gpio_attach_irq(gpio_dev, INTN_PIN, irqSBC_Handler, NULL);

    /* register handler routine for SBC_ISR_WK_WU */
    SBC_Register_Callback(SBC_ISR_WK_WU, irqSBC_Wake_Handler);

    /* register handler routine for SBC_ISR_CAN_WU */
    SBC_Register_Callback(SBC_ISR_CAN_WU, irqSBC_CAN_Wake_Handler);

    /* register handler routine for SBC_ISR_TIMER_WU */
    SBC_Register_Callback(SBC_ISR_TIMER_WU, irqSBC_Timer_Wake_Handler);

    /* register handler routine for SBC_ISR_GPIO_WK_WU */
    SBC_Register_Callback(SBC_ISR_GPIO_WK_WU, irqSBC_GPIO_Wake_Handler);

    /* register handler routine for SBC_ISR_VS_UV */
    SBC_Register_Callback(SBC_ISR_VS_UV, irqSBC_ISR_VS_UV_Handler);

    /* register handler routine for SBC_ISR_VS_OV */
    SBC_Register_Callback(SBC_ISR_VS_OV, irqSBC_ISR_VS_OV_Handler);

    /* register handler routine for SBC_ISR_VCC1_OV */
    SBC_Register_Callback(SBC_ISR_VCC1_OV, irqSBC_ISR_VCC1_OV_Handler);

    /* register handler routine for SBC_ISR_VCC1_UV_PREWARN */
    SBC_Register_Callback(SBC_ISR_VCC1_UV_PREWARN, irqSBC_ISR_VCC1_UV_PREWARN_Handler);

    /* register handler routine for SBC_ISR_VCC2_OT */
    SBC_Register_Callback(SBC_ISR_VCC2_OT, irqSBC_ISR_VCC2_OT_Handler);

    /* register handler routine for SBC_ISR_VCC2_UV */
    SBC_Register_Callback(SBC_ISR_VCC2_UV, irqSBC_ISR_VCC2_UV_Handler);

    /* register handler routine for SBC_ISR_VCC1_UV */
    SBC_Register_Callback(SBC_ISR_VCC1_UV, irqSBC_ISR_VCC1_UV_Handler);

    /* register handler routine for SBC_ISR_TSD2_SAFESTATE */
    SBC_Register_Callback(SBC_ISR_TSD2_SAFESTATE, irqSBC_ISR_TSD2_SAFESTATE_Handler);

    /* register handler routine for SBC_ISR_TSD1 */
    SBC_Register_Callback(SBC_ISR_TSD1, irqSBC_ISR_TSD1_Handler);

    /* register handler routine for SBC_ISR_TPW */
    SBC_Register_Callback(SBC_ISR_TPW, irqSBC_ISR_TPW_Handler);

    /* register handler routine for SBC_ISR_RESTART_AFTER_FAIL */
    SBC_Register_Callback(SBC_ISR_RESTART_AFTER_FAIL, irqSBC_ISR_RESTART_AFTER_FAIL_Handler);

    /* register handler routine for SBC_ISR_FROM_SLEEPMODE */
    SBC_Register_Callback(SBC_ISR_FROM_SLEEPMODE, irqSBC_ISR_FROM_SLEEPMODE_Handler);

    /* register handler routine for SBC_ISR_WD_FAIL_1 */
    SBC_Register_Callback(SBC_ISR_WD_FAIL_1, irqSBC_ISR_WD_FAIL_1_Handler);

    /* register handler routine for SBC_ISR_WD_FAIL_2 */
    SBC_Register_Callback(SBC_ISR_WD_FAIL_2, irqSBC_ISR_WD_FAIL_2_Handler);

    /* register handler routine for SBC_ISR_FAILURE */
    SBC_Register_Callback(SBC_ISR_FAILURE, irqSBC_ISR_FAILURE_Handler);

    /* register handler routine for SBC_ISR_CAN_TSD */
    SBC_Register_Callback(SBC_ISR_CAN_TSD, irqSBC_ISR_CAN_TSD_Handler);

    /* register handler routine for SBC_ISR_CAN_TXD_DOM_TO */
    SBC_Register_Callback(SBC_ISR_CAN_TXD_DOM_TO, irqSBC_ISR_CAN_TXD_DOM_TO_Handler);

    /* register handler routine for SBC_ISR_CAN_BUS_DOM_TO */
    SBC_Register_Callback(SBC_ISR_CAN_BUS_DOM_TO, irqSBC_ISR_CAN_BUS_DOM_TO_Handler);

    /* register handler routine for SBC_ISR_VCAN_UV */
    SBC_Register_Callback(SBC_ISR_VCAN_UV, irqSBC_ISR_VCAN_UV_Handler);

    /* register handler routine for SBC_ISR_GPIO_OC */
    SBC_Register_Callback(SBC_ISR_GPIO_OC, irqSBC_ISR_GPIO_OC_Handler);

    /* register handler routine for SBC_ISR_GPIO_OC */
    SBC_Register_Callback(SBC_ISR_GPIO_OL, irqSBC_ISR_GPIO_OL_Handler);


    printf("SBC Monitor Serve Create Success\r\n");

    return 0;
}

#define CAN_INFO 0

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

    while (true) {
        can_receive_buf(can_dev, &frame);

        printf("id=0x%x len=%d\n", frame.id, frame.len);
        for (int i = 0; i < frame.len; i++) {
            printf("data[%d]=0x%x ", i, frame.data[i]);
        }
        printf("\n");
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
can_device_t *sbc_can_dev;

static int can_init_demo(int argc, char *argv[])
{
    can_device_t *can_dev = can_get_probe_device(15);

    if (can_dev == NULL) {
        printf("CAN Controller 15 not Support \r\ncheck if can 16 is aveilabe\r\n");
        return 0;
    }

    sbc_can_dev=can_dev;

    can_create_tx_thread(sbc_can_dev);
    can_create_rx_thread(sbc_can_dev);

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

    if (argc != 4) {
        ssdk_printf(CAN_INFO, "\nargc must be 4");
        ssdk_printf(CAN_INFO,
                    "\t\t:can_demo write [mb_idx] [id] [fd] [count]\n");
        return 0;
    }

    mb_idx = strtoul(argv[0], NULL, 0);
    frame.id = strtoul(argv[1], NULL, 0);
    frame.fd = strtoul(argv[2], NULL, 0);

    if (frame.fd)
        frame.len = 64;
    else
        frame.len = 8;

    frame.ide = CAN_STDID;
    frame.rtr = CAN_DTR;
    memcpy(&frame.data, &Data, frame.len);

    count = strtoul(argv[3], NULL, 0);

    for (int i = 0; i < count; i++) {
        ret = can_write(sbc_can_dev, &frame, mb_idx);
        osDelay(1);
        if (ret) {
            ssdk_printf(CAN_INFO, "can write failed\n");
        }
    }

    return 0;
}

static int sbc_can_demo_main(int argc, char *argv[])
{
    if (!strcmp(argv[0], "init")) {
        can_init_demo(argc - 1, &argv[1]);
    } else if (!strcmp(argv[0], "write")) {
        can_write_data(argc - 1, &argv[1]);
    } else {
        ssdk_printf(CAN_INFO, "error cmd\n");
    }

    return CLI_FALSE;
}

static int sbc_stop_main(int argc, char *argv[])
{
    uint32_t onoff;

    if (argc == 1) {
        onoff = atoi(argv[0]);

        if (onoff) {
            SBC_Set_Mode_Normal();
            ssdk_printf(0, "set sbc to normal\n");
        }
        else {
            SBC_Mode_Stop_Without_WTD();
            ssdk_printf(0, "stop wdt, set sbc to stop\n");
        }
    }

    return CLI_FALSE;
}

CLI_CMD("sbc", "\r\nsbc: \r\n sbc debug cmd", sbc_main);
CLI_CMD("sbc_can", "\r\ncan_demo:\r\n can device demo\n", sbc_can_demo_main);
CLI_CMD("sbc_monitor_start", "\r\nstart sbc interrupt server",
        sbc_monitor_serve_start_main);
CLI_CMD("sbc_stop", "\r\nsbc_stop: \r\n sbc_stop", sbc_stop_main);
