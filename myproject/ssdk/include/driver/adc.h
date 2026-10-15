/**
 * @file adc.h
 * @brief adc api header.
 * @note    Application files cannot include this header file directly.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef ADC_H_
#define ADC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <compiler.h>
#include <stdarg.h>
#include <types.h>
#include <debug.h>

#include "cmsis_os2.h"
#include "driver/clk.h"


typedef uint16_t adc_data_size_t;
typedef uint16_t adc_ch_size_t;

typedef enum
{
    ADC_XFER_INTERRUPT_MODE_E = 0,
    ADC_XFER_POLLING_MODE_E,
    ADC_XFER_DMA_MODE_E,
} adc_xfer_mod_e_t;

/**
 * @brief adc basic configurations.
 */
typedef struct adc_config {
    uint32_t irq;
    addr_t base;
    uint8_t resolution;
    bool inter_ref_volt;
    uint32_t sample_freq;
    adc_xfer_mod_e_t xfer_mode;
    bool is_diff;
    clk_node_t *clk;
} adc_config_t;

typedef struct adc_channel_chain {
    uint8_t *chain_mask;
    uint8_t chain_mask_len;
} adc_channel_chain_t;

#ifdef ADC_MAX_CHANNEL_NUM
#define DEFINE_CHANNEL_MASK(name)                                           \
  uint8_t name##_array[ADC_MAX_CHANNEL_NUM / 8] = { 0 };                       \
  adc_channel_chain_t name = {                                              \
      .chain_mask = name##_array,                                           \
      .chain_mask_len = ADC_MAX_CHANNEL_NUM / 8,                            \
  }
#endif

typedef struct adc_buff {
    adc_data_size_t sample_val;
    adc_ch_size_t channel;
} __PACKED adc_buff_t;

typedef void (*adc_callback_t)(adc_buff_t *buff, uint16_t len, void *context);

struct adc_ops;

/**
 * @brief adc device.
 */
typedef struct adc_dev {
    adc_config_t    adc_conf;
    adc_buff_t      *buf;
    uint16_t        len;
    uint16_t        cur_len;
    uint16_t        fifo_size;
    adc_ch_size_t   channel_num;
    adc_callback_t  cb;
    void            *context;
    osMutexId_t     mutex;
    struct adc_ops  *ops;
    void *priv;
} adc_dev_t;

typedef struct adc_ops {
    int (*adc_start)(adc_dev_t *device, adc_channel_chain_t *channel_chain,
                     adc_buff_t *buf, uint16_t len);
    int (*adc_stop)(adc_dev_t *device);
    int (*adc_read_sample)(adc_dev_t *device, adc_channel_chain_t *channel_chain,
                           adc_buff_t *buff, uint16_t len);
    void (*adc_main_func)(adc_dev_t *device);
    int (*adc_irq_handler)(uint32_t irq, void *arg);
} adc_ops_t;

/**
 * @brief adc_add_channel_mask.
 *
 * @param[in] chain mask point
 * @param[in] number of configured channels
 * @param[in] channel num
 */
void adc_add_channel_mask(adc_channel_chain_t *name, adc_ch_size_t num, ...);

/**
 * @brief adc_clear_channel_mask.
 *
 * @param[in] chain mask point
 * @param[in] number of cleared channels
 * @param[in] channel num
 */
void adc_clear_channel_mask(adc_channel_chain_t *name, adc_ch_size_t num, ...);

/**
 * @brief adc_clear_all_channel_mask.
 *
 * @param[in] chain mask point
 */
void adc_clear_all_channel_mask(adc_channel_chain_t *name);

/**
 * @brief adc init.
 *
 * @param[in] adc dev
 * @param[in] adc config
 * @return int32_t
 * @retval ADC_OK: success
 * @retval ADC_ERR: fail
 */
int32_t adc_init(adc_dev_t *device, const adc_config_t *cfg);

/**
 * @brief adc deinit.
 *
 * @param[in] adc dev
 * @return int32_t
 * @retval ADC_OK: success
 * @retval ADC_ERR: fail
 */
int32_t adc_deinit(adc_dev_t *device);

/**
 * @brief Read Sample from multi channel (sync).
 *
 * @param[in] adc dev
 * @param[in] adc multi channel to read data
 * @param[out] adc multi channel read sample value
 * @param[in] buf len
 * @return int32_t
 * @retval ADC_OK: success
 * @retval ADC_ERR: fail
 */
int32_t adc_read_sample(adc_dev_t *device, adc_channel_chain_t *channel_chain,
                        adc_buff_t *buff, uint16_t len);

/**
 * @brief Continuous read Sample from multi channel.
 *
 * @param[in] adc dev
 * @param[in] adc multi channel to read data
 * @param[in] adc buf
 * @return int32_t
 * @retval ADC_OK: success
 * @retval ADC_ERR: fail
 */
int32_t adc_start(adc_dev_t *device, adc_channel_chain_t *channel_chain,
                  adc_buff_t *buf, uint16_t len);

/**
 * @brief Stop continuous read Sample from multi channel.
 *
 * @param[in] adc dev
 * @return int32_t
 * @retval ADC_OK: success
 * @retval ADC_ERR: fail
 */
int32_t adc_stop(adc_dev_t *device);

/**
 * @brief Set callback for async mode.
 *
 * @param[in] adc dev
 * @param[in] callback
 * @param[in] context
 */
static inline void adc_set_callback(adc_dev_t *device, adc_callback_t callback,
                      void *context)
{
    ASSERT(device);

    device->cb = callback;
    device->context = context;
}

/**
 * @brief main_func in polling mode.
 *
 * @param[in] adc dev
 */
void adc_main_func(adc_dev_t *device);

#ifdef __cplusplus
}
#endif

#endif /* _ADC_H */
