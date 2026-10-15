#include <board.h>
#include <clock_cfg.h>
#include <cmsis_os2.h>
#include <driver/taishan/adc/taishan_adc.h>
#include <driver/gpio.h>
#include <driver/taishan/pinctrl/taishan_pinctrl.h>
#include <param.h>
#include <reg.h>
#include <scr_cfg.h>
#include <scr_hw.h>
#include <stdio.h>
#include <string.h>

#include "CLI_type.h"
#include "irq_num.h"
#include "regs_base.h"

#define BOARD_ADC2_REF_VOLT 3300
#define GPIO_B12_CH_SEL                                                        \
    ADC_CH_SEL_TAISHAN(6, ADC_CH_POLAR_NEG, ADC2_CH6N_MUX_B12)

extern pinctrl_t g_pinctrl_safety;
extern gpio_dev_t g_gpio_safety;

static int analog_in_test(int argc, char *argv[])
{
    adc_dev_t dev;
    DEFINE_CHANNEL_MASK(channel);
    adc_buff_t adc_buff[10];

    adc_config_t adc_config = {
        .irq = DEVICE_INTR(SADC2_O_SADC),
        .base = DEVICE_BASE(ADC2),
        .resolution = 12,
        .inter_ref_volt = false,
        .sample_freq = 100 * 1000,
        .xfer_mode = ADC_XFER_INTERRUPT_MODE_E,
        .is_diff = false,
        .clk = CLK_NODE(g_ckgen_ip_adc2),
    };

    const scr_signal_t asd_scr[] = {SCR_SF_ANA_SF_CFG_APD_A_B12_CTRL_3_0};
    scr_set(&g_scr_sf, &asd_scr[0], 1);

    pinctrl_config(&g_pinctrl_safety, GPIO_B12, PINCTRL_CONFIG_MODE_SELECT,
                   PINCTRL_MODE_SELECT_NORMAL);
    taishan_adc_probe(&dev, &adc_config);

    adc_add_channel_mask(&channel, 1, GPIO_B12_CH_SEL);
    adc_read_sample(&dev, &channel, adc_buff, ARRAY_SIZE(adc_buff));

    for (int i = 0; i < 10; i++) {
        printf("ADC17 sample_val %d Volt=%dmV\n", adc_buff[i].sample_val,
               adc_buff[i].sample_val * BOARD_ADC2_REF_VOLT / 4096);
    }

    adc_deinit(&dev);

    return CLI_FALSE;
}

CLI_CMD("analog_in_demo", "\r\nanalog_in_test demo\r\n", analog_in_test);
