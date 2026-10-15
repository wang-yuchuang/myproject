#include <arch.h>
#include <cmsis_os2.h>
#include <board.h>
#include <driver/kunlun/adc/kunlun_adc.h>
#include <stdio.h>
#include <reg.h>
#include <string.h>
#include <param.h>

#include "regs_base.h"
#include "irq_num.h"
#include "CLI_type.h"

static void adc_callback(adc_buff_t *buff, uint16_t len, void *context)
{
    for (int i = 0; i < len; i++) {
        printf("channel: %d, sample_val: %d\n", buff[i].channel,
               buff[i].sample_val);
    }
}

static int adc_test(int argc, char *argv[])
{
    adc_dev_t dev;
    DEFINE_CHANNEL_MASK(channel);
    adc_buff_t adc_buff[10];
    int32_t ret;

    adc_config_t adc_config = {
        .irq            = DEVICE_INTR(ADC),
        .base           = DEVICE_BASE(ADC),
        .resolution     = 12,
        .inter_ref_volt = false,
        .sample_freq    = 50000,
        .xfer_mode    = ADC_XFER_INTERRUPT_MODE_E,
        .is_diff        = false,
    };

    ret = kunlun_adc_probe(&dev, &adc_config);
    if(ret != 0) {
        printf("adc_init error\n");
        return CLI_FALSE;
    }

    adc_add_channel_mask(&channel, 3, 0, 1, 3);
    ret = adc_read_sample(&dev, &channel, adc_buff, ARRAY_SIZE(adc_buff));
    if (ret != 0) {
        printf("adc_read_sample error\n");
        return CLI_FALSE;
    }

    for (int i = 0; i < ARRAY_SIZE(adc_buff); i++) {
        printf("channel: %d, sample_val: %d\n", adc_buff[i].channel,
               adc_buff[i].sample_val);
    }

    adc_deinit(&dev);

    /* async */
    ret = kunlun_adc_probe(&dev, &adc_config);
    if(ret != 0) {
        printf("adc_init error\n");
        return CLI_FALSE;
    }
    adc_set_callback(&dev, adc_callback, NULL);
    adc_clear_all_channel_mask(&channel);
    adc_add_channel_mask(&channel, 4, 4, 5, 6, 7);
    ret = adc_start(&dev, &channel, adc_buff, ARRAY_SIZE(adc_buff));
    if (ret != 0) {
        printf("adc_start error\n");
        return CLI_FALSE;
    }

    osDelay(1000);
    adc_stop(&dev);

    return CLI_FALSE;
}



/*
 * Register Internel CLI commands.
 */
CLI_CMD("adc_test", "\r\nadc_test unittest\r\n", adc_test);

