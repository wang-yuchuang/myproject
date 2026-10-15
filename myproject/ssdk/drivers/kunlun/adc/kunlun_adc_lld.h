/*
* dw_adc.h
*
* Copyright (c) 2020 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement semidrive fuse drv head file
*
*/

#ifndef KUNLUN_ADC_LLD_H_
#define KUNLUN_ADC_LLD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <param.h>
#include <reg.h>
#include <types.h>

#define ADC_MAX_ENTRIES_NUM (64)
#define ADC_MAX_FIFO_NUM    (128)

#define ADC_MAX_CLK_FREQ    (70000000u) // 70M
#define ADC_CLK_SRC_FREQ    (400000000u) // 400M
#define ADC_MAX_CLK_6_FREQ  (ADC_MAX_CLK_FREQ / 8)
#define ADC_MAX_CLK_8_FREQ  (ADC_MAX_CLK_FREQ / 10)
#define ADC_MAX_CLK_10_FREQ (ADC_MAX_CLK_FREQ / 12)
#define ADC_MAX_CLK_12_FREQ (ADC_MAX_CLK_FREQ / 14)

// ADC_CTRL
#define ADC_CTRL_BASE                    (0x0u)
#define ADC_CTRL_DWC_SW_RST_FIELD_OFFSET (17u)
#define ADC_CTRL_DWC_SW_RST_FIELD_SIZE   (1u)
#define ADC_CTRL_SW_RST_FIELD_OFFSET     (16u)
#define ADC_CTRL_SW_RST_FIELD_SIZE       (1u)
#define ADC_CTRL_DIFF_EN_FIELD_OFFSET    (5u)
#define ADC_CTRL_DIFF_EN_FIELD_SIZE      (1u)
#define ADC_CTRL_SELRES_FIELD_OFFSET     (3u)
#define ADC_CTRL_SELRES_FIELD_SIZE       (2u)
#define ADC_CTRL_SELBG_FIELD_OFFSET      (2u)
#define ADC_CTRL_SELBG_FIELD_SIZE        (1u)
#define ADC_CTRL_SELREF_FIELD_OFFSET     (1u)
#define ADC_CTRL_SELREF_FIELD_SIZE       (1u)
#define ADC_CTRL_ENADC_FIELD_OFFSET      (0u)
#define ADC_CTRL_ENADC_FIELD_SIZE        (1u)

// ADC_CLK_CFG
#define ADC_CLK_CFG_BASE                    (0x4u)
#define ADC_CLK_CFG_DWC_CLK_EN_FIELD_OFFSET (15u)
#define ADC_CLK_CFG_DWC_CLK_EN_FIELD_SIZE   (1u)
#define ADC_CLK_CFG_DIV_NUM_FIELD_OFFSET    (2u)
#define ADC_CLK_CFG_DIV_NUM_FIELD_SIZE      (8u)
#define ADC_CLK_CFG_SRC_SEL_FIELD_OFFSET    (0u)
#define ADC_CLK_CFG_SRC_SEL_FIELD_SIZE      (2u)

// ADC_IMASK
#define ADC_IMASK_BASE             (0x14u)
#define ADC_IMASK_WML_FIELD_OFFSET (1u)
#define ADC_IMASK_WML_FIELD_SIZE   (1u)
#define ADC_IMASK_EOC_FIELD_OFFSET (0u)
#define ADC_IMASK_EOC_FIELD_SIZE   (1u)

// ADC_IFLAG
#define ADC_IFLAG_BASE                 (0x18u)
#define ADC_IFLAG_DUMMY_FIELD_OFFSET   (5u)
#define ADC_IFLAG_DUMMY_FIELD_SIZE     (1u)
#define ADC_IFLAG_TMR_OVF_FIELD_OFFSET (4u)
#define ADC_IFLAG_TMR_OVF_FIELD_SIZE   (1u)
#define ADC_IFLAG_EOL_FIELD_OFFSET     (3u)
#define ADC_IFLAG_EOL_FIELD_SIZE       (1u)
#define ADC_IFLAG_CTC_FIELD_OFFSET     (2u)
#define ADC_IFLAG_CTC_FIELD_SIZE       (1u)
#define ADC_IFLAG_WML_FIELD_OFFSET     (1u)
#define ADC_IFLAG_WML_FIELD_SIZE       (1u)
#define ADC_IFLAG_EOC_FIELD_OFFSET     (0u)
#define ADC_IFLAG_EOC_FIELD_SIZE       (1u)

// ADC_SSC
#define ADC_SSC_BASE                    (0x20u)
#define ADC_SSC_INTERVAL_FIELD_OFFSET   (8u)
#define ADC_SSC_INTERVAL_FIELD_SIZE     (8u)
#define ADC_SSC_CONV_STOP_FIELD_OFFSET  (4u)
#define ADC_SSC_CONV_STOP_FIELD_SIZE    (1u)
#define ADC_SSC_CONV_START_FIELD_OFFSET (3u)
#define ADC_SSC_CONV_START_FIELD_SIZE   (1u)
#define ADC_SSC_CONV_INIT_FIELD_OFFSET  (2u)
#define ADC_SSC_CONV_INIT_FIELD_SIZE    (1u)
#define ADC_SSC_CONV_MODE_FIELD_OFFSET  (0u)
#define ADC_SSC_CONV_MODE_FIELD_SIZE    (2u)

// ADC_SCHC
#define ADC_SCHC_BASE              (0x28u)
#define ADC_SCHC_CTC_FIELD_OFFSET  (16u)
#define ADC_SCHC_CTC_FIELD_SIZE    (16u)
#define ADC_SCHC_CSEL_FIELD_OFFSET (0u)
#define ADC_SCHC_CSEL_FIELD_SIZE   (7u)

// ADC_MCHC
#define ADC_MCHC_BASE                  (0x2cu)
#define ADC_MCHC_REPEAT_FIELD_OFFSET   (8u)
#define ADC_MCHC_REPEAT_FIELD_SIZE     (4u)
#define ADC_MCHC_LOOP_END_FIELD_OFFSET (0u)
#define ADC_MCHC_LOOP_END_FIELD_SIZE   (6u)

// ADC_FIFO
#define ADC_FIFO_BASE                (0x30u)
#define ADC_FIFO_ENABLE_FIELD_OFFSET (31u)
#define ADC_FIFO_ENABLE_FIELD_SIZE   (1u)
#define ADC_FIFO_FLUSH_FIELD_OFFSET  (30u)
#define ADC_FIFO_FLUSH_FIELD_SIZE    (1u)
#define ADC_FIFO_THRE_FIELD_OFFSET   (16u)
#define ADC_FIFO_THRE_FIELD_SIZE     (8u)
#define ADC_FIFO_IDLE_FIELD_OFFSET   (10u)
#define ADC_FIFO_IDLE_FIELD_SIZE     (1u)
#define ADC_FIFO_FULL_FIELD_OFFSET   (9u)
#define ADC_FIFO_FULL_FIELD_SIZE     (1u)
#define ADC_FIFO_EMPTY_FIELD_OFFSET  (8u)
#define ADC_FIFO_EMPTY_FIELD_SIZE    (1u)
#define ADC_FIFO_WML_FIELD_OFFSET    (0u)
#define ADC_FIFO_WML_FIELD_SIZE      (8u)

// ADC_RSLT
#define ADC_RSLT_BASE                (0x100u)
#define ADC_RSLT_TS_IND_FIELD_OFFSET (31u)
#define ADC_RSLT_TS_IND_FIELD_SIZE   (1u)
#define ADC_RSLT_CSEL_FIELD_OFFSET   (16u)
#define ADC_RSLT_CSEL_FIELD_SIZE     (7u)
#define ADC_RSLT_VALUE_FIELD_OFFSET  (0u)
#define ADC_RSLT_VALUE_FIELD_SIZE    (16u)

// ADC_ENTRY
#define ADC_ENTRY_BASE                   (0x300u)
#define ADC_ENTRY_TSAMP_FIELD_OFFSET     (24u)
#define ADC_ENTRY_TSAMP_FIELD_SIZE       (8u)
#define ADC_ENTRY_ENABLE_FIELD_OFFSET    (23u)
#define ADC_ENTRY_ENABLE_FIELD_SIZE      (1u)
#define ADC_ENTRY_SKIP_FIELD_OFFSET      (22u)
#define ADC_ENTRY_SKIP_FIELD_SIZE        (1u)
#define ADC_ENTRY_DUMMY_FIELD_OFFSET     (21u)
#define ADC_ENTRY_DUMMY_FIELD_SIZE       (1u)
#define ADC_ENTRY_SETUP_SEL_FIELD_OFFSET (16u)
#define ADC_ENTRY_SETUP_SEL_FIELD_SIZE   (2u)
#define ADC_ENTRY_CSEL_FIELD_OFFSET      (8u)
#define ADC_ENTRY_CSEL_FIELD_SIZE        (7u)
#define ADC_ENTRY_RCT_FIELD_OFFSET       (0u)
#define ADC_ENTRY_RCT_FIELD_SIZE         (8u)

typedef enum {
    ADC_EOC_INT_FLAG_E1 = 0x01,
    ADC_WML_INT_FLAG_E2 = 0x02,
    ADC_CTC_INT_FLAG_E4 = 0x04,
    ADC_EOL_INT_FLAG_E8 = 0x08,
    ADC_OVF_INT_FLAG_E16 = 0x10,
    ADC_DUMMY_INT_FLAG_E32 = 0x20,
} adc_lld_int_flag_t;

typedef enum {
    ADC_SINGLE_CH_SINGLE_E0 = 0x00,
    ADC_SINGLE_CH_BACK2BACK_E1,
    ADC_SINGLE_CH_INTERVAL_E2,
    ADC_MULTIPLE_CH_E3,
} adc_lld_convert_mode_t;

typedef enum {
    ADC_6_BITS_E0 = 0x00,
    ADC_8_BITS_E1 = 0x01,
    ADC_10_BITS_E2 = 0x02,
    ADC_12_BITS_E3 = 0x03,
} adc_lld_resolution_e_t;

typedef enum {
    ADC_SINGLE_CONTINUONS_IRQ = 1,
    ADC_MULTI_CONTINUONS_IRQ,
    ADC_IRQ_MAX,
} adc_lld_iflag_t;

static inline void adc_lld_conv_stop(addr_t base)
{
    RMWREG32(base + ADC_SSC_BASE, ADC_SSC_CONV_STOP_FIELD_OFFSET,
           ADC_SSC_CONV_STOP_FIELD_SIZE, 1u);
}

static inline void adc_lld_set_repeat(addr_t base, int8_t repeat)
{
    RMWREG32(base + ADC_MCHC_BASE, ADC_MCHC_REPEAT_FIELD_OFFSET,
           ADC_MCHC_REPEAT_FIELD_SIZE, (repeat & 0xfu));
}

static inline int32_t adc_lld_stop(addr_t base, uint8_t channel)
{
    adc_lld_conv_stop(base);

    return 0;
}

static inline int32_t adc_lld_multi_stop(addr_t base)
{
    adc_lld_conv_stop(base);

    return 0;
}

static inline bool adc_lld_is_fifo_empty(addr_t base)
{
      uint32_t reg_val;

      reg_val = readl(base + ADC_FIFO_BASE);
      return ((reg_val & (1u << ADC_FIFO_EMPTY_FIELD_OFFSET)) ? true
                                                              : false);
}

static inline uint32_t adc_lld_get_fifo_wml(addr_t base)
{
    return (readl(base + ADC_FIFO_BASE) & 0x7fu);
}

static inline void adc_lld_set_fifo_wml(addr_t base, uint8_t wml_val)
{
    if (wml_val > ADC_MAX_FIFO_NUM) {
        wml_val = ADC_MAX_FIFO_NUM;
    }

    RMWREG32(base + ADC_FIFO_BASE, ADC_FIFO_THRE_FIELD_OFFSET,
             ADC_FIFO_THRE_FIELD_SIZE, wml_val);
}

static inline void adc_lld_set_clk_freq(addr_t base, uint32_t freq)
{
    RMWREG32(base + ADC_CLK_CFG_BASE, ADC_CLK_CFG_DIV_NUM_FIELD_OFFSET,
             ADC_CLK_CFG_DIV_NUM_FIELD_SIZE,
             DIV_ROUND_UP(ADC_CLK_SRC_FREQ, freq));
}

static inline int32_t adc_lld_set_resolution(addr_t base,
                                             uint8_t u8_resolution)
{
    int32_t ret = 0;
    uint8_t reg_val = 0;

    switch (u8_resolution) {
    case 6:
        reg_val = ADC_6_BITS_E0;
        break;
    case 8:
        reg_val = ADC_8_BITS_E1;
        break;
    case 10:
        reg_val = ADC_10_BITS_E2;
        break;
    case 12:
        reg_val = ADC_12_BITS_E3;
        break;
    default:
        ret = -1;
        break;
    }

    RMWREG32(base + ADC_CTRL_BASE, ADC_CTRL_SELRES_FIELD_OFFSET,
             ADC_CTRL_SELRES_FIELD_SIZE, reg_val);

  return ret;
}

static inline void adc_lld_sw_rst(addr_t base, bool is_enable)
{
    if (is_enable) {
        RMWREG32(base + ADC_CTRL_BASE, ADC_CTRL_SW_RST_FIELD_OFFSET,
                 ADC_CTRL_SW_RST_FIELD_SIZE, 1u);
    } else {
        RMWREG32(base + ADC_CTRL_BASE, ADC_CTRL_SW_RST_FIELD_OFFSET,
                 ADC_CTRL_SW_RST_FIELD_SIZE, 0u);
    }
}

static inline void adc_lld_set_inter_ref_volt(addr_t base, bool is_inter)
{
    if (is_inter) {
        RMWREG32(base + ADC_CTRL_BASE, ADC_CTRL_SELBG_FIELD_OFFSET,
                 ADC_CTRL_SELBG_FIELD_SIZE, 1u);
        RMWREG32(base + ADC_CTRL_BASE, ADC_CTRL_SELREF_FIELD_OFFSET,
                 ADC_CTRL_SELREF_FIELD_SIZE, 1u);
    } else {
        RMWREG32(base + ADC_CTRL_BASE, ADC_CTRL_SELBG_FIELD_OFFSET,
                 ADC_CTRL_SELBG_FIELD_SIZE, 0u);
        RMWREG32(base + ADC_CTRL_BASE, ADC_CTRL_SELREF_FIELD_OFFSET,
                 ADC_CTRL_SELREF_FIELD_SIZE, 0u);
    }
}

static inline int32_t adc_lld_set_sample_freq(addr_t base, uint32_t sample_freq,
                                              uint8_t resolution)
{
    int32_t ret = 0;
    uint8_t internal = 0;

    switch (resolution) {
    case 6:
        internal = sample_freq < ADC_MAX_CLK_6_FREQ
                       ? DIV_ROUND_UP(ADC_MAX_CLK_FREQ, sample_freq) - 1
                       : 0;
        break;
    case 8:
        internal = sample_freq < ADC_MAX_CLK_8_FREQ
                       ? DIV_ROUND_UP(ADC_MAX_CLK_FREQ, sample_freq) - 1
                       : 0;
        break;
    case 10:
        internal = sample_freq < ADC_MAX_CLK_10_FREQ
                       ? DIV_ROUND_UP(ADC_MAX_CLK_FREQ, sample_freq) - 1
                       : 0;
        break;
    case 12:
        internal = sample_freq < ADC_MAX_CLK_12_FREQ
                       ? DIV_ROUND_UP(ADC_MAX_CLK_FREQ, sample_freq) - 1
                       : 0;
        break;
    default:
        ret = -1;
        break;
    }

    RMWREG32(base + ADC_SSC_BASE, ADC_SSC_INTERVAL_FIELD_OFFSET,
           ADC_SSC_INTERVAL_FIELD_SIZE, internal);

    return ret;
}

static inline void adc_lld_diff_en(addr_t base, uint8_t channel,
                                   bool is_enable)
{
    if (is_enable) {
        RMWREG32(base + ADC_CTRL_BASE, ADC_CTRL_DIFF_EN_FIELD_OFFSET,
                 ADC_CTRL_DIFF_EN_FIELD_SIZE, 1u);
    } else {
        RMWREG32(base + ADC_CTRL_BASE, ADC_CTRL_DIFF_EN_FIELD_OFFSET,
                 ADC_CTRL_DIFF_EN_FIELD_SIZE, 0u);
    }
}

static inline void adc_lld_en(addr_t base, bool is_enable)
{
    if (is_enable) {
        RMWREG32(base + ADC_CTRL_BASE, ADC_CTRL_ENADC_FIELD_OFFSET,
                 ADC_CTRL_ENADC_FIELD_SIZE, 1u);
    } else {
        RMWREG32(base + ADC_CTRL_BASE, ADC_CTRL_ENADC_FIELD_OFFSET,
                 ADC_CTRL_ENADC_FIELD_SIZE, 0u);
    }
}

static inline void adc_lld_dwc_sw_rst(addr_t base, bool is_enable)
{
    if (is_enable) {
        RMWREG32(base + ADC_CTRL_BASE, ADC_CTRL_DWC_SW_RST_FIELD_OFFSET,
                 ADC_CTRL_DWC_SW_RST_FIELD_SIZE, 1u);
    } else {
        RMWREG32(base + ADC_CTRL_BASE, ADC_CTRL_DWC_SW_RST_FIELD_OFFSET,
                 ADC_CTRL_DWC_SW_RST_FIELD_SIZE, 0u);
    }
}

static inline void adc_lld_en_fifo(addr_t base, bool is_enable)
{
    if (is_enable) {
        RMWREG32(base + ADC_FIFO_BASE, ADC_FIFO_ENABLE_FIELD_OFFSET,
                 ADC_FIFO_ENABLE_FIELD_SIZE, 1u);
    } else {
        RMWREG32(base + ADC_FIFO_BASE, ADC_FIFO_ENABLE_FIELD_OFFSET,
                 ADC_FIFO_ENABLE_FIELD_SIZE, 0u);
    }
}

static inline void adc_lld_conv_init(addr_t base)
{
    RMWREG32(base + ADC_SSC_BASE, ADC_SSC_CONV_INIT_FIELD_OFFSET,
             ADC_SSC_CONV_INIT_FIELD_SIZE, 1u);
}

static inline void adc_lld_disable_all_int(addr_t base)
{
    RMWREG32(base + ADC_IMASK_BASE, 0, 6, 0x3fu);
}

static inline void adc_lld_set_channel(addr_t base, uint8_t channel)
{
    RMWREG32(base + ADC_SCHC_BASE, ADC_SCHC_CSEL_FIELD_OFFSET,
             ADC_SCHC_CSEL_FIELD_SIZE, (channel & 0x7fu));
}

static inline void adc_lld_single_ch_single_init(addr_t base,
                              adc_lld_convert_mode_t convert_mode)
{
    adc_lld_set_repeat(base, 1);

    RMWREG32(base + ADC_SSC_BASE, ADC_SSC_CONV_MODE_FIELD_OFFSET,
             ADC_SSC_CONV_MODE_FIELD_SIZE, convert_mode);
}

static inline void adc_lld_flush_fifo(addr_t base)
{
    RMWREG32(base + ADC_FIFO_BASE, ADC_FIFO_FLUSH_FIELD_OFFSET,
             ADC_FIFO_FLUSH_FIELD_SIZE, 1u);
}

static inline void adc_lld_conv_start(addr_t base)
{
    RMWREG32(base + ADC_SSC_BASE, ADC_SSC_CONV_START_FIELD_OFFSET,
             ADC_SSC_CONV_START_FIELD_SIZE, 1u);
}

static inline uint32_t adc_lld_get_fifo_reg_value(addr_t base)
{
    return readl(base + ADC_RSLT_BASE);
}

static inline int32_t adc_lld_get_channel_data(uint8_t channel,
                                               uint32_t result_data,
                                               uint16_t *sample_val)
{
    if (!sample_val) {
        return -1;
    }

    if ((((result_data >> ADC_RSLT_CSEL_FIELD_OFFSET) & 0x7f) != channel) ||
      (result_data & (1u << ADC_RSLT_TS_IND_FIELD_OFFSET))) {
        return -1;
    }

    *sample_val = (uint16_t)(result_data & 0xffffu);

    return 0;
}

static inline void adc_lld_enable_all_int(addr_t base)
{
    RMWREG32(base + ADC_IMASK_BASE, 0, 6, 0x0u);
}

static inline void adc_lld_set_convert_mode(addr_t base,
                         adc_lld_convert_mode_t convert_mode)
{
    RMWREG32(base + ADC_SSC_BASE, ADC_SSC_CONV_MODE_FIELD_OFFSET,
             ADC_SSC_CONV_MODE_FIELD_SIZE, convert_mode);
}

static inline int32_t adc_lld_get_data(uint32_t result_data, uint8_t *channel,
                                       uint16_t *sample_val)
{
    if (!channel || !sample_val) {
        return -1;
    }

    if (result_data & (1u << ADC_RSLT_TS_IND_FIELD_OFFSET)) {
        return -1;
    }

    *sample_val = (uint16_t)(result_data & 0xffffu);
    *channel = ((result_data >> ADC_RSLT_CSEL_FIELD_OFFSET) & 0x7fu);

    return 0;
}

static inline void adc_lld_enable_int(addr_t base, uint32_t intb)
{
    uint32_t reg_val = readl(base + ADC_IMASK_BASE);

    reg_val &= ~intb;

    writel(reg_val, base + ADC_IMASK_BASE);
}

static inline void adc_lld_set_entry_channel(addr_t base, uint8_t n,
                                             uint8_t channel)
{
    RMWREG32(base + ADC_ENTRY_BASE + n * 4, ADC_ENTRY_CSEL_FIELD_OFFSET,
             ADC_ENTRY_CSEL_FIELD_SIZE, (channel & 0x7fu));
}

static inline void adc_lld_set_entry_en(addr_t base, uint8_t n, bool is_enable)
{
    if (is_enable) {
        RMWREG32(base + ADC_ENTRY_BASE + n * 4, ADC_ENTRY_ENABLE_FIELD_OFFSET,
                 ADC_ENTRY_ENABLE_FIELD_SIZE, 1u);
    } else {
        RMWREG32(base + ADC_ENTRY_BASE + n * 4, ADC_ENTRY_ENABLE_FIELD_OFFSET,
                 ADC_ENTRY_ENABLE_FIELD_SIZE, 0u);
    }
}

static inline void adc_lld_set_entry_skip(addr_t base, uint8_t n,
                                          bool is_enable)
{
    if (is_enable) {
        RMWREG32(base + ADC_ENTRY_BASE + n * 4, ADC_ENTRY_SKIP_FIELD_OFFSET,
                 ADC_ENTRY_SKIP_FIELD_SIZE, 1u);
    } else {
        RMWREG32(base + ADC_ENTRY_BASE + n * 4, ADC_ENTRY_SKIP_FIELD_OFFSET,
                 ADC_ENTRY_SKIP_FIELD_SIZE, 0u);
    }
}

static inline void adc_lld_set_entry(addr_t base, uint8_t n, uint32_t val)
{
    writel(val, base + ADC_ENTRY_BASE + n * 4);
}

static inline void adc_lld_set_entry_tsamp(addr_t base, uint8_t n,
                                           uint32_t cycle_num)
{
    RMWREG32(base + ADC_ENTRY_BASE + n * 4, ADC_ENTRY_TSAMP_FIELD_OFFSET,
             ADC_ENTRY_TSAMP_FIELD_SIZE, (cycle_num & 0xffu));
}

static inline uint32_t adc_lld_get_iflag(addr_t base)
{
    return readl(base + ADC_IFLAG_BASE);
}

static inline void adc_clear_int_bits(addr_t base, uint32_t int_bits)
{
    writel(int_bits, base + ADC_IFLAG_BASE);
}

static inline void adc_clear_all_int_flag(addr_t base_addr)
{
    adc_clear_int_bits(base_addr,
                       ADC_EOC_INT_FLAG_E1 | ADC_WML_INT_FLAG_E2 |
                       ADC_CTC_INT_FLAG_E4 | ADC_EOL_INT_FLAG_E8 |
                       ADC_OVF_INT_FLAG_E16 | ADC_DUMMY_INT_FLAG_E32);
}


#ifdef __cplusplus
}
#endif

#endif
