/**
 * @file snd_driver.h
 * @version 0.1
 * @date 2022-03-02
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 *
 */

#ifndef SND_DRIVER_H_
#define SND_DRIVER_H_

#include <cmsis_os2.h>
#include <debug.h>
#include <driver/clk.h>
#include <stdbool.h>
#include <types.h>
#include <reg.h>

#include "driver/dma.h"

typedef struct snd_dev snd_dev_t;

#define SND_DAIFMT_FORMAT_MASK 0x0f

/* i2s standard phillips interface mode */
#define SND_DAIFMT_STANDARD_PHILLIPS 1
/* i2s left justified interface mode */
#define SND_DAIFMT_LEFT_JUSTIFIED 2
/* i2s right justified interface mode */
#define SND_DAIFMT_RIGHT_JUSTIFIED 3
/* i2s dsp mode-A */
#define SND_DAIFMT_DSP_A 4
/* i2s dsp mode-B */
#define SND_DAIFMT_DSP_B 5
#define SND_DAIFMT_AC97 6
/*PDM */
#define SND_DAIFMT_PDM 7

/* transfer control mode master or slave */
#define SND_DAIFMT_MASTER_MASK (0x0f0)
#define SND_DAIFMT_CODEC_SLAVE (1 << 4)
#define SND_DAIFMT_CODEC_MASTER (2 << 4)

#define SND_DAIFMT_INV_MASK 0x0f00
#define SND_DAIFMT_INV_OFFSET 8
/* normal bit clock + frame */
#define SND_DAIFMT_NB_NF (0 << SND_DAIFMT_INV_OFFSET)
/* normal BCLK + inv FRM */
#define SND_DAIFMT_NB_IF (SND_DAIFMT_INV_OFFSET)

/* invert BCLK + nor FRM  */
#define SND_DAIFMT_IB_NF (3 << SND_DAIFMT_INV_OFFSET)

/* invert BCLK + FRM */
#define SND_DAIFMT_IB_IF (4 << SND_DAIFMT_INV_OFFSET)

typedef enum snd_channel {
    SND_CHANNEL_NUM_MONO = 1,
    SND_CHANNEL_NUM_STEREO = 2,
    SND_CHANNEL_NUM_3CHANS = 3,
    SND_CHANNEL_NUM_4CHANS = 4,
    SND_CHANNEL_NUM_5CHANS = 5,
    SND_CHANNEL_NUM_6CHANS = 6,
    SND_CHANNEL_NUM_7CHANS = 7,
    SND_CHANNEL_NUM_8CHANS = 8,
    SND_CHANNEL_NUM_9CHANS = 9,
    SND_CHANNEL_NUM_10CHANS = 10,
    SND_CHANNEL_NUM_11CHANS = 11,
    SND_CHANNEL_NUM_12CHANS = 12,
    SND_CHANNEL_NUM_13CHANS = 13,
    SND_CHANNEL_NUM_14CHANS = 14,
    SND_CHANNEL_NUM_15CHANS = 15,
    SND_CHANNEL_NUM_16CHANS = 16,
} snd_channel_t;

typedef enum snd_sample_rate {
    SND_SR_NO_INIT = 0,
    SND_SR_5512 = 5512,
    SND_SR_8000 = 8000,
    SND_SR_11025 = 11025,
    SND_SR_16000 = 16000,
    SND_SR_22050 = 22050,
    SND_SR_32000 = 32000,
    SND_SR_44100 = 44100,
    SND_SR_48000 = 48000,
    SND_SR_64000 = 64000,
    SND_SR_88200 = 88200,
    SND_SR_96000 = 96000,
    SND_SR_176400 = 176400,
    SND_SR_192000 = 192000,
} snd_sample_rate_t;

/* sample width */
typedef enum snd_sample_width {
    SND_SAMPLE_WIDTH_8BITS = 8,
    SND_SAMPLE_WIDTH_10BITS = 10,
    SND_SAMPLE_WIDTH_12BITS = 12,
    SND_SAMPLE_WIDTH_14BITS = 14,
    SND_SAMPLE_WIDTH_16BITS = 16,
    SND_SAMPLE_WIDTH_20BITS = 20,
    SND_SAMPLE_WIDTH_24BITS = 24,
    SND_SAMPLE_WIDTH_32BITS = 32,
} snd_sample_width_t;

/* slot width */
typedef enum snd_slot_width {
    SND_SLOT_WIDTH_8BITS = 8,
    SND_SLOT_WIDTH_10BITS = 10,
    SND_SLOT_WIDTH_12BITS = 12,
    SND_SLOT_WIDTH_14BITS = 14,
    SND_SLOT_WIDTH_16BITS = 16,
    SND_SLOT_WIDTH_20BITS = 20,
    SND_SLOT_WIDTH_24BITS = 24,
    SND_SLOT_WIDTH_32BITS = 32,
} snd_slot_width_t;

typedef enum {
    SND_STREAM_PLAYBACK = 0,
    SND_STREAM_CAPTURE,
} snd_stream_type_t;

typedef struct snd_dma_buf {
    uint8_t *dma_addr;
    paddr_t phy_addr; /// unused
    int32_t dma_size; ///< real lenght buffer_bytes
    int32_t period_size; // period_bytes
    int32_t period_count; //periods
    uint32_t appl_ptr; ///< user ptr
    uint32_t hw_ptr;   ///< dma ptr
    uint32_t boundary; ///< unit dmasize
} snd_dma_buf_t;

typedef enum snd_state {
    /*
     * The interface was initialized but is not yet ready to receive /
     * its state to READY.
     */
    SND_STATE_NOT_READY,
    /* The interface is ready to receive / transmit data. */
    SND_STATE_READY,
    /* The interface is receiving / transmitting data. */
    SND_STATE_RUNNING,
    /* The interface is STOPING. */
    SND_STATE_STOPPING,
    /* The interface is pasuing */
    SND_STATE_PAUSING,
    /* TX buffer underrun or RX buffer overrun has occurred. */
    SND_STATE_OVERRUN,
    /* TX buffer underrun or RX buffer underrun has occurred. */
    SND_STATE_UNDERRUN,
} snd_state_t;

typedef void (*snd_callback)(void *context, snd_state_t state);

typedef enum snd_trigger_op {
    SND_TRIGGER_START,
    SND_TRIGGER_STOP,
    SND_TRIGGER_PAUSE,
    SND_TRIGGER_RESUME,
} snd_trigger_op_t;

typedef struct snd_params {
    uint32_t fmt;
    snd_channel_t channels;
    snd_sample_width_t sample_width;
    snd_sample_rate_t sample_rate;
    unsigned int period_size;
    unsigned int period_count;
    snd_channel_t slots;
    snd_slot_width_t slot_width;
    unsigned int tx_slot_mask;
    unsigned int rx_slot_mask;
    /* xrun recover */
    bool xrun_enable;
    uint32_t start_threshold;
    uint32_t stop_threshold;
} snd_params_t;

typedef struct snd_dma_data {
    paddr_t dma_addr;
    unsigned int addr_width;
    unsigned int maxburst;
    unsigned int fifo_size;
} snd_dma_data_t;

typedef struct snd_ops {
    bool (*startup)(snd_dev_t *dev, snd_stream_type_t stream);
    bool (*set_params)(snd_dev_t *dev, snd_params_t *params,
                       snd_stream_type_t stream);
    bool (*set_tdm_slot)(snd_dev_t *dev, unsigned int tx_mask,
                         unsigned int rx_mask, unsigned int slots,
                         snd_slot_width_t slot_width);
    bool (*trigger)(snd_dev_t *dev, snd_trigger_op_t snd_cmd,
                    snd_stream_type_t stream);
    bool (*shutdown)(snd_dev_t *dev, snd_stream_type_t stream);
} snd_ops_t;

typedef struct snd_stream {
    snd_params_t params;
    snd_dma_data_t dma_data;
    snd_dma_buf_t dma_buf;
    dma_chan_t *dma_ch;
    snd_state_t state;
    snd_callback callback; /* app callback */
    void *context;         /* callback arg */
} snd_stream_t;

struct snd_dev {
    paddr_t base; /* snd reg base */
    clk_node_t *clk;
    int irq; /* interrput number */
    dma_dev_t *dma_dev;
    osMutexId_t dev_lock[2];
    snd_stream_t stream[2];
    snd_ops_t *ops;
    void *priv;
    osEventFlagsId_t wr_event;
    osEventFlagsId_t rd_event;
};

/**
 * @brief sound write register
 *
 * @param[in] base base_addr
 * @param[in] reg  offset_addr
 * @param[in] val  vaulue
 */
static inline void snd_write_reg(paddr_t base, uint32_t reg,
                                        uint32_t val)
{
    writel(val, base + reg);
}

/**
 * @brief sound read register
 *
 * @param[in] base base_addr
 * @param[in] reg  offset_addr
 * @return vaule
 */
static inline uint32_t snd_read_reg(paddr_t base, uint32_t reg)
{
    return readl(base + reg);
}

/**
 * @brief ssoundupdate register bits
 *
 * @param[in] base base_addr
 * @param[in] reg  offset_addr
 * @param[in] mask bit mask
 * @param[in] val  value mask
 */
static inline void snd_update_bits(paddr_t base, uint32_t reg,
                                           uint32_t mask, uint32_t val)
{
    uint32_t temp = snd_read_reg(base, reg);

    temp &= ~mask;
    temp |= val;

    snd_write_reg(base, reg, temp);
}

/**
 * @brief get dma buf available size
 *
 * @param[in] dev sound device
 * @param[in] stream  stream type
 *
 * @return available size
 */
int32_t snd_get_hw_avail(snd_dev_t *dev,snd_stream_type_t stream);

/**
 * @brief
 *
 * @param dev
 * @param params
 * @param callback
 * @param context
 * @param stream
 * @return true
 * @return false
 */
bool snd_open(snd_dev_t *dev, snd_params_t *params, snd_callback callback,
              void *context, snd_stream_type_t stream);
/**
 * @brief
 *
 * @param dev
 * @param stream
 * @return true
 * @return false
 */
bool snd_close(snd_dev_t *dev, snd_stream_type_t stream);
/**
 * @brief
 *
 * @param dev
 * @param stream
 * @return true
 * @return false
 */
bool snd_start(snd_dev_t *dev, snd_stream_type_t stream);
/**
 * @brief
 *
 * @param dev
 * @param stream
 * @return true
 * @return false
 */
bool snd_stop(snd_dev_t *dev, snd_stream_type_t stream);
/**
 * @brief
 *
 * @param dev
 * @param stream
 * @return true
 * @return false
 */
bool snd_pause(snd_dev_t *dev, snd_stream_type_t stream);
/**
 * @brief
 *
 * @param dev
 * @param stream
 * @return true
 * @return false
 */
bool snd_resume(snd_dev_t *dev, snd_stream_type_t stream);
/**
 * @brief
 *
 * @param dev
 * @param buffer
 * @param len
 * @return int
 */
int snd_write(snd_dev_t *dev, uint8_t *buffer, uint32_t len);
/**
 * @brief
 *
 * @param dev
 * @param buffer
 * @param len
 * @return int
 */
int snd_read(snd_dev_t *dev, uint8_t *buffer, uint32_t len);

void snd_dev_init(snd_dev_t *dev);

void snd_dev_deinit(snd_dev_t *dev);
#endif /* SND_DRIVER_H_ */
