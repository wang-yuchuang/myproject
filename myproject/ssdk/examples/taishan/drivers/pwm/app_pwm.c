/*
 * @File: app_pwm.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * @Descripttion:
 *
 * @Revision History:
 * ----------------
 */

#define LOG_TAG "app_pwm.c"
#include <arch/cache.h>
#include <debug.h>
#include <driver/gpio.h>
#include <driver/pinctrl.h>
#include <driver/taishan/pinctrl/taishan_pinctrl.h>
#include <irq_num.h>
#include <param.h>
#include <regs_base.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>

#include "CLI.h"
#include "board.h"
#include "dma_cfg.h"
#include "driver/dma.h"
#include "driver/irq.h"
#include "driver/taishan/epwm/sdrv_epwm_drv.h"
#include "driver/taishan/xtrg/sdrv_xtrg_drv.h"
#include "pwm_cfg.h"
#include "reg.h"

extern dma_dev_t dma_sf0;
#define PWM_OUT1 9  // GPIO_A9
#define PWM_OUT2 10 // GPIO_A10
#define total_array_number 32
#define total_array_number_byte (total_array_number * 4)

enum test_choose_num {
    /* 0  */ PWM_ONE_CHANL_OUTPUT,
    /* 1  */ PWM_COMPLEMENTARY_OUTPUT,
    /* 2  */ PWM_WITH_DMA_OUTPUT,
    /* 3  */ PWM_CMP_INT_WITH_XTRG,
};
static uint32_t src_data_array[total_array_number]
    __attribute__((__aligned__(32))) = {0};
static uint32_t *dma_test_src = &src_data_array[0];
static struct dma_dev_cfg dma_cfg = {0};
static dma_chan_t *chan = NULL;
static pwm_state_t test_state_1 = {
    .period = 1000000,
    .duty = 250000,
};
static xtrg_dev_t pwm_test_xtrg_dev_g = {
    .base = APB_XTRG_BASE,
    .irq = XTRG_FUNC_INTR_NUM,
};
static xtrg_config_t pwm_test_xtrg_config_g;
extern gpio_dev_t g_gpio_safety;
extern gpio_dev_t g_gpio_ap;
static gpio_dev_t *gpio_dev;
static uint32_t pin = GPIO_S0;
/*static uint32_t test1 = PWM_PERIOD_CYCLES / 8;
static uint32_t test2 = PWM_PERIOD_CYCLES / 8;
static uint32_t test3 = PWM_PERIOD_CYCLES / 8;*/
#define GPIO_INSTANCE(pin)                                                     \
    (((pin) < TAISHAN_SAFETY_PIN_NUM) ? &g_gpio_safety : &g_gpio_ap)

void pwm_one_chanl_output(void)
{
    /*xtrg cfg*/
    memset(&pwm_test_xtrg_config_g, 0, sizeof(pwm_test_xtrg_config_g));

    pwm_test_xtrg_config_g.sig_i = EPWM_CMP0(0);
    pwm_test_xtrg_config_g.sig_o = IO_o(PWM_OUT1);
    sdrv_xtrg_io_out_config(&pwm_test_xtrg_dev_g, &pwm_test_xtrg_config_g,
                            true);

    pwm_config(g_pwm_1, &test_state_1);
    pwm_start(g_pwm_1);
}

void pwm_complementary_output(void)
{
    sdrv_epwm_cmp_prefin_pol_t prefin_pol = {
        .cmp0_pol = SDRV_EPWM_POL_KEEP,
        .cmp1_pol = SDRV_EPWM_POL_KEEP,
    };

    memset(&pwm_test_xtrg_config_g, 0, sizeof(pwm_test_xtrg_config_g));

    pwm_test_xtrg_config_g.sig_i = EPWM_CMP_B0(0);
    pwm_test_xtrg_config_g.sig_o = IO_o(PWM_OUT1);
    sdrv_xtrg_io_out_config(&pwm_test_xtrg_dev_g, &pwm_test_xtrg_config_g,
                            true);

    pwm_test_xtrg_config_g.sig_i = EPWM_CMP_B1(0);
    pwm_test_xtrg_config_g.sig_o = IO_o(PWM_OUT2);
    sdrv_xtrg_io_out_config(&pwm_test_xtrg_dev_g, &pwm_test_xtrg_config_g,
                            true);

    pwm_config(g_pwm_2, &test_state_1);
    sdrv_epwm_dti(g_pwm_2, &prefin_pol, 1500);
    pwm_start(g_pwm_2);
}

static void app_pwm_with_dma_handler(uint32_t status, uint32_t param,
                                     void *context)
{
    /*do sth*/
}

void pwm_with_dma_output(void)
{
    PLOGI("In");

    uint8_t tr_ret = 0;

    for (uint16_t i = 0; i < 32; i++) {
        static uint16_t index = 0;
        index = i % 8;
        dma_test_src[i] = 25000 * (10 - (index + 1));
    }

    dma_flush_cache((addr_t)dma_test_src,
                    ROUNDUP(total_array_number_byte, CONFIG_ARCH_CACHE_LINE));

    chan = dma_req_chan(&dma_sf0);

    if (NULL == chan) {
        PLOGI("alloc channel failed.\r\n");
    }

    chan->irq_cb = app_pwm_with_dma_handler;
    chan->context = NULL;
    dma_cfg.direction = DMA_MEM2DEV;
    dma_cfg.src_addr = (addr_t)dma_test_src;
    dma_cfg.dst_addr = APB_EPWM1_BASE + EPWM_FIFO_C_OFFSET; //  fifo address
    dma_cfg.src_addr_width = DMA_DEV_BUSWIDTH_4_BYTES;
    dma_cfg.dst_addr_width = DMA_DEV_BUSWIDTH_4_BYTES;
    dma_cfg.src_maxburst = 8;
    dma_cfg.dst_maxburst = 8;

    dma_config_chan(chan, &dma_cfg);

    tr_ret = dma_prep_cyclic(chan, dma_test_src, 128, 32, DMA_INTERRUPT);

    if (0 == tr_ret) {
        PLOGI("set transfer config err.\r\n");
    }

    dma_start(chan);

    memset(&pwm_test_xtrg_config_g, 0, sizeof(pwm_test_xtrg_config_g));
    pwm_test_xtrg_config_g.sig_i = EPWM_CMP_C0(0);
    pwm_test_xtrg_config_g.sig_o = IO_o(PWM_OUT1);
    sdrv_xtrg_io_out_config(&pwm_test_xtrg_dev_g, &pwm_test_xtrg_config_g,
                            true);

    pwm_config(g_pwm_3, &test_state_1);
    pwm_start(g_pwm_3);
}

void xtrg_fault_event_handler(void *arg, sdrv_xtrg_int_event_e event)
{
    if (event == FAULT_EVENT) {
        writel(0x1, APB_XTRG_BASE + FUNC_SMON_CMP_ERR_INT_STA);
        gpio_set(gpio_dev, pin, 1);
        udelay(10);
        gpio_set(gpio_dev, pin, 0);
    }
}

void pwm_cmp_int_with_xtrg(void)
{
    PLOGI("In");
    gpio_dev = GPIO_INSTANCE(pin);
    sdrv_epwm_input_sel_t ext_sig_cfg = {
        .set_sel = SDRV_EPWM_INPUT_SEL_TRIG_CMP_A_CE,
        .clr_sel = SDRV_EPWM_INPUT_SEL_TRIG_CMP_A_CE,
    };

    sdrv_epwm_lld_ext_output_sel(APB_EPWM1_BASE, &ext_sig_cfg);
    sdrv_epwm_lld_cmp_config_ce_en(APB_EPWM1_BASE, SDRV_EPWM_CHANNEL_A, true);

    xtrg_swt_config_t swt_cfg;

    /*XTRG SW TRIG CFG*/
    swt_cfg.swt = XTRG_SWT1;
    swt_cfg.width = 0xff;
    swt_cfg.event = XTRG_SWT_EVENT_CLEAR;
    sdrv_xtrg_swt_config(&pwm_test_xtrg_dev_g, &swt_cfg);

    writel(0x1, APB_XTRG_BASE + FUNC_SMON_CMP_ERR_INT_STA_EN);
    writel(1 << 3, APB_XTRG_BASE + FAULT_SOURCE_EN2);

    xtrg_smon_config_t xtrg_smon_config_0;
    xtrg_smon_config_0.sig_in_sel = EPWM_SET_I(0);
    xtrg_smon_config_0.sig_input0_sel = 0;
    xtrg_smon_config_0.sig_input1_sel = 1;
    xtrg_smon_config_0.change_pol_in0 = 0;
    xtrg_smon_config_0.smon_sel = XTRG_SMON0;
    sdrv_xtrg_smon_config(&pwm_test_xtrg_dev_g, &xtrg_smon_config_0, true);

    xtrg_smon_config_t xtrg_smon_config_1;
    xtrg_smon_config_1.sig_in_sel = XTRG_SWT(XTRG_SWT1);
    xtrg_smon_config_1.smon_sel = XTRG_SMON1;
    sdrv_xtrg_smon_config(&pwm_test_xtrg_dev_g, &xtrg_smon_config_1, true);

    xtrg_callback_t xtrg_fault_event_handler_cb = xtrg_fault_event_handler;
    xtrg_set_cb(&pwm_test_xtrg_dev_g, xtrg_fault_event_handler_cb, NULL);
    sdrv_xtrg_init(&pwm_test_xtrg_dev_g);
    sdrv_xtrg_int_enable(&pwm_test_xtrg_dev_g, 1 << 0);

    pwm_one_chanl_output();
}

static int pwm_app_main(int argc, char *argv[])
{
    PLOGI("In");

    uint8_t test_num = 0;
    if (argc > 0)
        test_num = atoi(argv[0]);

    switch (test_num) {
    case PWM_ONE_CHANL_OUTPUT:
        pwm_one_chanl_output();
        break;
    case PWM_COMPLEMENTARY_OUTPUT:
        pwm_complementary_output();
        break;
    case PWM_WITH_DMA_OUTPUT:
        pwm_with_dma_output();
        break;
    case PWM_CMP_INT_WITH_XTRG:
        pwm_cmp_int_with_xtrg();
        break;
    default:
        PLOGI("invalid_test_num");
        break;
    }
    return 0;
}

CLI_CMD("epwm_app", "\r\npwm_app:\r\n Start PWM APP\r\n", pwm_app_main);
