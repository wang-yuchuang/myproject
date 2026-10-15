/*
 * power_monitor.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Board level monitor monitor example.
 *
 * Revision History:
 * -----------------
 */

#include <stdio.h>
#include <string.h>
#include <param.h>
#include <reg.h>
#include <board.h>
#include <clock_cfg.h>
#include <cmsis_os2.h>
#include <driver/taishan/adc/taishan_adc.h>
#include <driver/gpio.h>
#include <driver/taishan/pinctrl/taishan_pinctrl.h>
#include <driver/scr.h>
#include <scr_cfg.h>
#include <scr_hw.h>

#include "power_monitor.h"

#include "CLI_type.h"


#define BOARD_ADC_REF_VOLT 3300


struct analog_scr {
    unsigned int pin;
    scr_signal_t sig;
};

static const struct analog_scr analog_scr_signals[] = {
    {GPIO_A0, SCR_SF_ANA_SF_CFG_APD_A_A0_CTRL_3_0},
    {GPIO_A1, SCR_SF_ANA_SF_CFG_APD_A_A1_CTRL_3_0},
    {GPIO_A2, SCR_SF_ANA_SF_CFG_APD_A_A2_CTRL_3_0},
    {GPIO_A3, SCR_SF_ANA_SF_CFG_APD_A_A3_CTRL_3_0},
    {GPIO_A4, SCR_SF_ANA_SF_CFG_APD_A_A4_CTRL_3_0},
    {GPIO_A5, SCR_SF_ANA_SF_CFG_APD_A_A5_CTRL_3_0},
    {GPIO_A6, SCR_SF_ANA_SF_CFG_APD_A_A6_CTRL_3_0},
    {GPIO_A7, SCR_SF_ANA_SF_CFG_APD_A_A7_CTRL_3_0},
    {GPIO_A8, SCR_SF_ANA_SF_CFG_APD_A_A8_CTRL_3_0},
    {GPIO_A9, SCR_SF_ANA_SF_CFG_APD_A_A9_CTRL_3_0},
    {GPIO_A10, SCR_SF_ANA_SF_CFG_APD_A_A10_CTRL_3_0},
    {GPIO_A11, SCR_SF_ANA_SF_CFG_APD_A_A11_CTRL_3_0},
    {GPIO_A12, SCR_SF_ANA_SF_CFG_APD_A_A12_CTRL_3_0},
    {GPIO_A13, SCR_SF_ANA_SF_CFG_APD_A_A13_CTRL_3_0},
    {GPIO_A14, SCR_SF_ANA_SF_CFG_APD_A_A14_CTRL_3_0},
    {GPIO_A15, SCR_SF_ANA_SF_CFG_APD_A_A15_CTRL_3_0},
    {GPIO_B0, SCR_SF_ANA_SF_CFG_APD_A_B0_CTRL_3_0},
    {GPIO_B1, SCR_SF_ANA_SF_CFG_APD_A_B1_CTRL_3_0},
    {GPIO_B2, SCR_SF_ANA_SF_CFG_APD_A_B2_CTRL_3_0},
    {GPIO_B3, SCR_SF_ANA_SF_CFG_APD_A_B3_CTRL_3_0},
    {GPIO_B4, SCR_SF_ANA_SF_CFG_APD_A_B4_CTRL_3_0},
    {GPIO_B5, SCR_SF_ANA_SF_CFG_APD_A_B5_CTRL_3_0},
    {GPIO_B6, SCR_SF_ANA_SF_CFG_APD_A_B6_CTRL_3_0},
    {GPIO_B7, SCR_SF_ANA_SF_CFG_APD_A_B7_CTRL_3_0},
    {GPIO_B8, SCR_SF_ANA_SF_CFG_APD_A_B8_CTRL_3_0},
    {GPIO_B9, SCR_SF_ANA_SF_CFG_APD_A_B9_CTRL_3_0},
    {GPIO_B10, SCR_SF_ANA_SF_CFG_APD_A_B10_CTRL_3_0},
    {GPIO_B11, SCR_SF_ANA_SF_CFG_APD_A_B11_CTRL_3_0},
    {GPIO_B12, SCR_SF_ANA_SF_CFG_APD_A_B12_CTRL_3_0},
    {GPIO_B13, SCR_SF_ANA_SF_CFG_APD_A_B13_CTRL_3_0},
    {GPIO_B14, SCR_SF_ANA_SF_CFG_APD_A_B14_CTRL_3_0},
    {GPIO_B15, SCR_SF_ANA_SF_CFG_APD_A_B15_CTRL_3_0},
    {GPIO_C0, SCR_SF_ANA_SF_CFG_APD_A_C0_CTRL_3_0},
    {GPIO_C1, SCR_SF_ANA_SF_CFG_APD_A_C1_CTRL_3_0},
    {GPIO_C2, SCR_SF_ANA_SF_CFG_APD_A_C2_CTRL_3_0},
    {GPIO_C3, SCR_SF_ANA_SF_CFG_APD_A_C3_CTRL_3_0},
    {GPIO_C4, SCR_SF_ANA_SF_CFG_APD_A_C4_CTRL_3_0},
    {GPIO_C5, SCR_SF_ANA_SF_CFG_APD_A_C5_CTRL_3_0},
    {GPIO_C6, SCR_SF_ANA_SF_CFG_APD_A_C6_CTRL_3_0},
    {GPIO_C7, SCR_SF_ANA_SF_CFG_APD_A_C7_CTRL_3_0},
    {GPIO_C8, SCR_SF_ANA_SF_CFG_APD_A_C8_CTRL_3_0},
    {GPIO_C9, SCR_SF_ANA_SF_CFG_APD_A_C9_CTRL_3_0},
    {GPIO_C10, SCR_SF_ANA_SF_CFG_APD_A_C10_CTRL_3_0},
    {GPIO_C11, SCR_SF_ANA_SF_CFG_APD_A_C11_CTRL_3_0},
    {GPIO_C12, SCR_SF_ANA_SF_CFG_APD_A_C12_CTRL_3_0},
    {GPIO_C13, SCR_SF_ANA_SF_CFG_APD_A_C13_CTRL_3_0},
    {GPIO_C14, SCR_SF_ANA_SF_CFG_APD_A_C14_CTRL_3_0},
    {GPIO_C15, SCR_SF_ANA_SF_CFG_APD_A_C15_CTRL_3_0},
};

static adc_dev_t g_adc_dev;

static adc_config_t adc_config = {
    .resolution = 12,
    .inter_ref_volt = false,
    .sample_freq = 100 * 1000,
    .xfer_mode = ADC_XFER_INTERRUPT_MODE_E,
    .is_diff = false,
};

extern struct voltage_channel board_voltage_channels[];
extern pinctrl_t g_pinctrl_safety;

static struct voltage_channel *find_voltage_channel(const char *name,
                                                    bool diff_n)
{
    for (struct voltage_channel *chnl = &board_voltage_channels[0];
         chnl->name != NULL; chnl++) {
        if ((diff_n == chnl->diff_n) && !strcmp(chnl->name, name))
            return chnl;
    }

    return NULL;
}

static const scr_signal_t *find_analog_scr(unsigned int pin)
{
    for (size_t i = 0; i < ARRAY_SIZE(analog_scr_signals); i++) {
        if (analog_scr_signals[i].pin == pin)
            return &analog_scr_signals[i].sig;
    }

    return NULL;
}

static bool read_voltage(struct voltage_channel *vol_chnl,
                         unsigned int *voltage)
{
    DEFINE_CHANNEL_MASK(channel);
    adc_buff_t adc_buff[10];
    const scr_signal_t *scr;
    int32_t ret;
    uint32_t sum;
    int i;

    /* Configure SCR to select analog channel for the pad. */
    scr = find_analog_scr(vol_chnl->pin);
    if (scr == NULL) {
        printf("Invalid ADC pin: %d\n", vol_chnl->pin);
        return false;
    }

    scr_set(&g_scr_sf, scr, 1);

    /* Configure pinmux. */
    pinctrl_config(&g_pinctrl_safety, vol_chnl->pin, PINCTRL_CONFIG_MODE_SELECT,
                   PINCTRL_MODE_SELECT_NORMAL);

    switch (vol_chnl->adc) {
    case 1:
        adc_config.irq = DEVICE_INTR(SADC1_O_SADC);
        adc_config.base = DEVICE_BASE(ADC1);
        adc_config.clk = CLK_NODE(g_ckgen_ip_adc1);
        break;

    case 2:
        adc_config.irq = DEVICE_INTR(SADC2_O_SADC);
        adc_config.base = DEVICE_BASE(ADC2);
        adc_config.clk = CLK_NODE(g_ckgen_ip_adc2);
        break;

    default:
        printf("ERROR. Add adc channel %d to monitor app.\n", vol_chnl->adc);
        return false;
    }

    if (taishan_adc_probe(&g_adc_dev, &adc_config)) {
        printf("Can't probe ADC\n");
        return false;
    }

    adc_add_channel_mask(&channel, 1, vol_chnl->adc_chnl);

    if (vol_chnl->prepare)
        vol_chnl->prepare(vol_chnl);

    ret = adc_read_sample(&g_adc_dev, &channel, adc_buff, ARRAY_SIZE(adc_buff));
    adc_deinit(&g_adc_dev);

    if (ret != 0) {
        printf("adc_read_sample error\n");
        return false;
    }

    sum  = 0;
    for (i = 1; i < 9; i++)
        sum += adc_buff[i].sample_val;

    *voltage = sum * BOARD_ADC_REF_VOLT / (4096 * 8);
    if (vol_chnl->ratio_mul && vol_chnl->ratio_div)
        *voltage = *voltage * vol_chnl->ratio_mul / vol_chnl->ratio_div;

    return true;
}

static int monitor_voltage(char *power)
{
    struct voltage_channel *vol_chnl;
    unsigned int vol; /* mV */

    vol_chnl = find_voltage_channel(power, false);
    if (!vol_chnl) {
        printf("Invalid voltage channel: %s\n", power);
        return -1;
    }

    if (!read_voltage(vol_chnl, &vol)) {
        printf("Can't get voltage for channel: %s\n", power);
        return -1;
    } else {
        printf("%s Voltage: %d mV\n", power, vol);
    }

    return 0;
}

static int monitor_current(char *power)
{
    printf("Current monitor not implemented!");
    return -1;
}

static int power_monitor_main(int argc, char *argv[])
{
    char *mode = argv[0];
    char *power = argv[1];

    printf("Power monitor %s %s\n", mode, power);

    if (!strcmp(mode, "vol")) {
        monitor_voltage(power);
    } else if (!strcmp(mode, "cur")) {
        monitor_current(power);
    }

    return CLI_FALSE;
}

CLI_CMD("power_mon", "\r\npower_mon:\r\n Board power monitor\n",
        power_monitor_main);

