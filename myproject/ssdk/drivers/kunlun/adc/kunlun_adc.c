/**
 * @file kunlun_adc.c
 * @brief Kunlun ADC driver implementation.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <string.h>
#include <debug.h>
#include <driver/adc.h>
#include <driver/irq.h>
#include <types.h>

#include "cmsis_os2.h"
#include "kunlun_adc_lld.h"

typedef enum {
    ADC_IDLE_E = 0,
    ADC_CONTINUOUS_RUNNING_E,
    ADC_MULTI_CONTINUONS_RUNNING_E,
} adc_state_t;

typedef struct adc_op {
    adc_state_t adc_state;
    adc_ch_size_t multi_channel_num;
} adc_op_t;

static adc_op_t g_adc_op = {.adc_state = ADC_IDLE_E,
                            .multi_channel_num = 0};


static void adc_hw_multi_channel_init(addr_t base, uint8_t *p_chain_mask,
                                      uint8_t chain_mask_len)
{
    uint8_t cur_entry = 0;

    ASSERT(p_chain_mask);

    adc_lld_set_repeat(base, 1);

    for (int i = 0; i < chain_mask_len; i++) {
        if (p_chain_mask[i]) {
            for (int j = 0; j < 8; j++) {
                if ((1u << j) & p_chain_mask[i]) {
                    adc_lld_set_entry_en(base, cur_entry, true);
                    adc_lld_set_entry_skip(base, cur_entry, true);
                    adc_lld_set_entry_channel(base, cur_entry, i * 8 + j);
                    cur_entry++;
                }
            }
        }
    }
}

static int32_t adc_hw_init(addr_t base, const adc_config_t *cfg)
{
    int32_t ret = 0;

    ASSERT(cfg);

    adc_lld_set_clk_freq(base, ADC_MAX_CLK_FREQ);

    ret = adc_lld_set_resolution(base, cfg->resolution);
    if (0 != ret) {
        return ret;
    }

    adc_lld_sw_rst(base, true);
    adc_lld_sw_rst(base, false);

    adc_lld_set_inter_ref_volt(base, cfg->inter_ref_volt);

    adc_lld_set_sample_freq(base, cfg->sample_freq, cfg->resolution);
    adc_lld_diff_en(base, 0, cfg->is_diff);

    adc_lld_en(base, true);
    adc_lld_dwc_sw_rst(base, true);
    adc_lld_dwc_sw_rst(base, false);

    adc_lld_en_fifo(base, true);
    adc_lld_conv_init(base);

    return ret;
}

static void adc_hw_deinit(addr_t base)
{
    adc_lld_sw_rst(base, true);
    adc_lld_sw_rst(base, false);

    adc_lld_dwc_sw_rst(base, true);
    adc_lld_dwc_sw_rst(base, false);

    adc_lld_en(base, 0);
}

static int32_t adc_hw_read_sample(addr_t base, uint8_t channel,
                            uint16_t *sample_val)
{
    uint32_t result_data;
    int ret = 0;

    ASSERT(sample_val);

    adc_lld_disable_all_int(base);

    adc_lld_set_channel(base, channel);

    adc_lld_single_ch_single_init(base, ADC_SINGLE_CH_INTERVAL_E2);

    adc_lld_flush_fifo(base);

    adc_lld_conv_start(base);

    while (!adc_lld_get_fifo_wml(base));

    result_data = adc_lld_get_fifo_reg_value(base);

    ret = adc_lld_get_channel_data(channel, result_data, sample_val);

    adc_lld_enable_all_int(base);

    return ret;
}

static int32_t adc_hw_read_multi_sample(addr_t base, uint8_t *chain_mask,
                                        uint8_t chain_mask_len, void *buf,
                                        uint16_t len)
{
    int32_t ret = 0;
    uint32_t result;
    uint8_t channel = 0;
    uint16_t sample_data = 0;
    adc_buff_t *buf_tmp;

    ASSERT(chain_mask);
    ASSERT(buf);

    adc_lld_disable_all_int(base);

    adc_hw_multi_channel_init(base, chain_mask, chain_mask_len);

    adc_lld_set_convert_mode(base, ADC_MULTIPLE_CH_E3);

    adc_lld_flush_fifo(base);

    adc_lld_conv_start(base);

    buf_tmp = (adc_buff_t *)buf;
    for (int i = 0; i < len; i++) {
        while (!adc_lld_get_fifo_wml(base));
        result = adc_lld_get_fifo_reg_value(base);
        adc_lld_get_data(result, &channel, &sample_data);
        buf_tmp[i].channel = channel;
        buf_tmp[i].sample_val = sample_data;
    }

    adc_lld_enable_all_int(base);

    return ret;
}

static int32_t adc_hw_start(addr_t base, uint8_t channel)
{
    int ret = 0;

    adc_lld_disable_all_int(base);

    adc_lld_enable_int(base, ADC_WML_INT_FLAG_E2);

    adc_lld_single_ch_single_init(base, ADC_SINGLE_CH_INTERVAL_E2);

    adc_lld_set_channel(base, channel);

    adc_lld_flush_fifo(base);

    adc_lld_conv_start(base);

    return ret;
}

static int32_t adc_hw_multi_start(addr_t base, uint8_t *chain_mask,
                                  uint8_t chain_mask_len)
{
    int32_t ret = 0;

    ASSERT(chain_mask);

    adc_lld_disable_all_int(base);

    adc_lld_enable_int(base, ADC_EOL_INT_FLAG_E8);

    adc_hw_multi_channel_init(base, chain_mask, chain_mask_len);

    adc_lld_set_convert_mode(base, ADC_MULTIPLE_CH_E3);

    adc_lld_flush_fifo(base);

    adc_lld_conv_start(base);

    return ret;
}

static int32_t adc_hw_get_result(addr_t base, uint8_t *channel,
                                 uint16_t *result)
{
    uint32_t result_tmp;

    ASSERT(channel);
    ASSERT(result);

    result_tmp = adc_lld_get_fifo_reg_value(base);

    if (adc_lld_get_data(result_tmp, channel, result) != 0) {
        return -1;
    }

    return 0;
}

static int32_t adc_hw_get_multi_result(addr_t base, void *buff, uint16_t len,
                                       uint16_t *cur_len)
{
    uint16_t tmp_len;
    uint32_t result;
    uint8_t channel = 0;
    uint16_t sample_data = 0;
    uint32_t *p_buf;

    ASSERT(buff);
    ASSERT(cur_len);

    tmp_len = adc_lld_get_fifo_wml(base) / 2;
    tmp_len = (tmp_len + *cur_len) > len ? (len - *cur_len) : tmp_len;

    p_buf = (uint32_t *)buff;

    for (uint16_t i = 0; i < tmp_len; i++) {
        /* hw bug. repeat at least twice */
        result = adc_lld_get_fifo_reg_value(base);
        adc_lld_get_fifo_reg_value(base);
        adc_lld_get_data(result, &channel, &sample_data);
        p_buf[(*cur_len)++] = (channel << 16) | sample_data;
    }

    return 0;
}

static uint32_t adc_hw_get_irq_status(addr_t base)
{
    uint32_t status;

    status = adc_lld_get_iflag(base);

    adc_clear_int_bits(base, status);

    if (status & ADC_EOL_INT_FLAG_E8) {
        return ADC_MULTI_CONTINUONS_IRQ;
    } else if (status & ADC_WML_INT_FLAG_E2) {
        return ADC_SINGLE_CONTINUONS_IRQ;
    }

    return ADC_IRQ_MAX;
}

static adc_ch_size_t adc_get_channel_num(adc_channel_chain_t *channel_chain,
                                         adc_ch_size_t *last_ch)
{
    uint8_t channel_num = 0;

    if (!channel_chain || !channel_chain->chain_mask || !last_ch) {
        return 0;
    }

    for (int i = 0; i < channel_chain->chain_mask_len; i++) {
        if (0 != channel_chain->chain_mask[i]) {
            for (int j = 0; j < 8; j++) {
                if (channel_chain->chain_mask[i] & (1UL << j)) {
                    channel_num++;
                    if (last_ch) {
                        *last_ch = i * 8 + j;
                    }
                }
            }
        }
    }

    return channel_num;
}

static void adc_handle_single_continous_irq(adc_dev_t *device)
{
    adc_ch_size_t channel = 0;
    adc_data_size_t result = 0;

    if (!device || !device->buf) {
        return;
    }

    if (ADC_CONTINUOUS_RUNNING_E != g_adc_op.adc_state) {
        return;
    }

    for (int i = 0; i < device->fifo_size; i++) {
        adc_hw_get_result(device->adc_conf.base, (uint8_t *)&channel, &result);
        device->buf[device->cur_len].channel = channel;
        device->buf[device->cur_len].sample_val = result;
        device->cur_len++;
        if (device->cur_len >= device->len) {
            device->cur_len = 0;
            if (device->cb) {
              device->cb(device->buf, device->len, device->context);
            }
        }
    }
}

static void adc_handle_multi_continous_irq(adc_dev_t *device)
{
    if (!device || !device->buf) {
        return;
    }

    if (ADC_MULTI_CONTINUONS_RUNNING_E != g_adc_op.adc_state) {
        return;
    }

    adc_hw_get_multi_result(device->adc_conf.base, (uint32_t *)device->buf,
                            device->len, &device->cur_len);

    if (device->cur_len >= device->len) {
        device->cur_len = 0;
        if (device->cb) {
            device->cb(device->buf, device->len, device->context);
        }
    }
}

static int kunlun_adc_irq_handler(uint32_t irq, void *arg)
{
    uint32_t status;
    adc_dev_t *device;

    if (!arg) {
        return -1;
    }

    device = (adc_dev_t *)arg;

    status = adc_hw_get_irq_status(device->adc_conf.base);

    switch (status) {
    case ADC_SINGLE_CONTINUONS_IRQ:
        adc_handle_single_continous_irq(device);
        break;
    case ADC_MULTI_CONTINUONS_IRQ:
        adc_handle_multi_continous_irq(device);
        break;
    default:
        break;
    }

    return 0;
}

static int32_t kunlun_adc_start(adc_dev_t *device, adc_channel_chain_t *channel_chain,
                         adc_buff_t *buf, uint16_t len)
{
    int32_t ret = 0;
    adc_ch_size_t channel;
    adc_ch_size_t ch_num;

    ASSERT(device);

    ret = adc_hw_init(device->adc_conf.base, &device->adc_conf);
    if (0 != ret) {
        return ret;
    }

    device->cur_len = 0;
    device->fifo_size = len > ADC_MAX_FIFO_NUM ? ADC_MAX_FIFO_NUM : len;
    device->buf = buf;
    device->len = len;

    adc_lld_set_fifo_wml(device->adc_conf.base, len);
    ch_num = adc_get_channel_num(channel_chain, &channel);

    if (1 == ch_num) {
        adc_hw_start(device->adc_conf.base, (uint8_t)channel);
        g_adc_op.adc_state = ADC_CONTINUOUS_RUNNING_E;
    } else {
        adc_hw_multi_start(device->adc_conf.base, channel_chain->chain_mask,
                         channel_chain->chain_mask_len);
        g_adc_op.adc_state = ADC_MULTI_CONTINUONS_RUNNING_E;
        g_adc_op.multi_channel_num = ch_num;
    }

    return ret;
}

static int32_t kunlun_adc_stop(adc_dev_t *device)
{
    int32_t ret = 0;

    ASSERT(device);

    adc_lld_multi_stop(device->adc_conf.base);

    adc_hw_deinit(device->adc_conf.base);

    g_adc_op.adc_state = ADC_IDLE_E;

    return ret;
}

static int32_t kunlun_adc_read_sample(adc_dev_t *device,
                               adc_channel_chain_t *channel_chain,
                               adc_buff_t *buff, uint16_t len)
{
    int32_t ret = 0;
    adc_ch_size_t ch_num = 0;
    adc_ch_size_t channel = 0;

    ASSERT(device);
    ASSERT(channel_chain);
    ASSERT(buff);

    ret = adc_hw_init(device->adc_conf.base, &device->adc_conf);
    if (0 != ret) {
        return -1;
    }

    ch_num = adc_get_channel_num(channel_chain, &channel);

    if (1 == ch_num) {
        ret = adc_hw_read_sample(device->adc_conf.base, (uint8_t)channel,
                               (uint16_t *)&buff->sample_val);
        buff[0].channel = channel;
    } else {
        ret = adc_hw_read_multi_sample(device->adc_conf.base,
                                     channel_chain->chain_mask,
                                     channel_chain->chain_mask_len,
                                     (void *)buff, len);
    }

    adc_hw_deinit(device->adc_conf.base);

    return ret;
}

static void kunlun_adc_main_func(adc_dev_t *device)
{
    if (ADC_XFER_POLLING_MODE_E == device->adc_conf.xfer_mode) {
        if (ADC_CONTINUOUS_RUNNING_E == g_adc_op.adc_state) {
            if (!adc_lld_is_fifo_empty(device->adc_conf.base)) {
                adc_handle_single_continous_irq(device);
            }
        } else if (ADC_MULTI_CONTINUONS_RUNNING_E == g_adc_op.adc_state) {
            if (adc_lld_get_fifo_wml(device->adc_conf.base) >=
                g_adc_op.multi_channel_num) {
                adc_handle_multi_continous_irq(device);
            }
        }
    }
}

static adc_ops_t adc_ops = {
    .adc_start = kunlun_adc_start,
    .adc_stop = kunlun_adc_stop,
    .adc_read_sample = kunlun_adc_read_sample,
    .adc_main_func = kunlun_adc_main_func,
    .adc_irq_handler = kunlun_adc_irq_handler,
};

int kunlun_adc_probe(adc_dev_t *device, const adc_config_t *cfg)
{
    ASSERT(device);

    adc_init(device, cfg);
    device->ops = &adc_ops;

    return 0;
}

