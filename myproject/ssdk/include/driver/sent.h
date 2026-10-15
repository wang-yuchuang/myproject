/**
 * @file sent.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef SENT_H
#define SENT_H

#include <types.h>
#include <compiler.h>
#include <cmsis_os2.h>
#include <lib/ring_buffer.h>

/* recv frame over callback func */
typedef void(*sent_recv_cb)(void *arg);

typedef struct sent_dev   sent_dev_t;
typedef struct sent_chan sent_chan_t;

typedef enum {
    SENT_STATUS_UNINIT = 0,
    SENT_STATUS_INITED
} sent_dev_sta_e;

/**
 * @enum Sent_ChannelStatusType
*/
typedef enum {
    SENT_STATUS_UNINITIALIZED = 0,
    SENT_STATUS_INITIALIZED = 1,
    SENT_STATUS_RUNNING = 2,
    SENT_STATUS_STOPED = 3,
} sent_chan_sta_e;

typedef enum{
    SENT_OK,
    SENT_SYNC_ERR = (1 << 0), /* Sync/Calibration pulse is not 56 ticks. */
    SENT_STATUS_ERR = (1 << 1), /* Status signal error. */
    SENT_DATA_ERR = (1 << 2), /* Nibble data is bigger than 15. */
    SENT_CRC_ERR = (1 << 3), /* Nibble data is bigger than 15. */
    SENT_LOW_PULSE_ERR = (1 << 4), /* Nibble low pulse width error. */
    SENT_MSB_NOT_ZERO = (1 << 5), /* SENT_HIGH_SPEED_FORMAT Only. */
    SENT_INV_MSN_ERR = (1 << 6), /* SENT_SINGLE_SECURE_FORMAT Only. */
    SENT_CONUNTER_VAL_ERR = (1 << 7), /* SENT_SINGLE_SECURE_FORMAT Only. */
    SENT_TICK_FREQ_ERR = (1 << 8), /* SENT_SINGLE_SECURE_FORMAT Only. */
    SENT_NOT_OK = (1 << 31), /* SENT_SINGLE_SECURE_FORMAT Only. */
}sent_err_t;

typedef enum{
    SENT_IDLE,
    SENT_SYNC,
    SENT_STATUS,
    SENT_DATA,
    SENT_CRC,
    SENT_PAUSE
} sent_state_e;

/* sdrv sent chan transmit format */
typedef enum {
    SENT_STANDARD_FORMAT,
    SENT_SINGLE_SECURE_FORMAT,
    SENT_HIGH_SPEED_FORMAT
} sent_fastchan_e;

/* sdrv sent data width */
typedef enum {
    SENT_DATA_WIDTH_4_BIT = 1,
    SENT_DATA_WIDTH_8_BIT,
    SENT_DATA_WIDTH_12_BIT,
    SENT_DATA_WIDTH_16_BIT,
    SENT_DATA_WIDTH_20_BIT,
    SENT_DATA_WIDTH_24_BIT,
} sent_bitnum_e;

/* sent operation */
typedef struct sent_ops {
    void (*init)(sent_dev_t *dev, uint8_t sdrv_sent_chanid);
    void (*deinit)(sent_dev_t *dev, uint8_t sdrv_sent_chanid);
    void (*recv_start)(sent_dev_t *dev, uint8_t sent_chan_id);
    void (*recv_stop)(sent_dev_t *dev, uint8_t sent_chan_id);
    // void (*msg_parse)(sent_dev_t *dev, uint8_t sent_chan_id);
    void (*frameinfo_get)(sent_dev_t *dev, uint8_t sent_chan_id);
    uint32_t (*get_datanum)(sent_dev_t *dev, uint8_t sdrv_sent_chanid);
    uint32_t (*get_data)(sent_dev_t *dev, uint8_t sdrv_sent_chanid);
} sent_ops_t;

/* device struct */
typedef struct sent_dev {
    paddr_t                 base;
    uint32_t                irq;
    osMutexId_t             mutex;

    sent_dev_sta_e          dev_sta;
    uint8_t                 chan_num;
    sent_chan_t             *chan;

    const sent_ops_t        *ops;
    void                    *priv;
} sent_dev_t;

/* channel struct */
typedef struct sent_chan {
    // sent_dev_t               *dev;
    uint32_t                chan_id;
    bool                    used;
    sent_chan_sta_e         chan_sta;

    osMutexId_t             mutex;

    sent_recv_cb            frameover_cb;
    void                    *arg;

    sent_fastchan_e          fast_chan;
    sent_bitnum_e            data_bitnum;
    uint8_t                  lowpulse_width;
    bool                     pause_pulse;

    uint32_t                expect_scaler;
    uint32_t                actual_scaler;

    uint8_t                 rx_index;
    uint8_t                 frame_dl;
    uint8_t                 frame_sta;
    uint8_t                 rxframe[6];
    uint8_t                 frame_crc;
    sent_state_e            cur_state;
    uint32_t                frame_err;

    void                     *priv;
} sent_chan_t;

/**
 * @brief sent init
 *
 * @param[in] dev sent dev
 */
void sent_init(sent_dev_t *dev);

/**
 * @brief sent deinit
 *
 * @param[in] dev sent dev
 */
void sent_deinit(sent_dev_t *dev);

/**
 * @brief sent start recv frame
 *
 * @param[in] dev sent dev
 * @param[in] sent_chan_id sent channel id
 */
void sent_recv_start(sent_dev_t *dev, uint8_t sent_chan_id);

/**
 * @brief sent stop recv frame
 *
 * @param[in] dev sent dev
 * @param[in] sent_chan_id sent channel id
 */
void sent_recv_stop(sent_dev_t *dev, uint8_t sent_chan_id);

/**
 * @brief parse sent frame
 *
 * @param[in] dev sent dev
 * @param[in] sent_chan_id sent channel id
 */
void sent_parse_msg(sent_dev_t *dev, uint8_t sent_chan_id);

/**
 * @brief sent set callback function
 *
 * @param[in] dev sent dev
 * @param[in] sent_chan_id sent channel id
 * @param[in] callback callback function
 * @param[in] arg callback arg
 */
void sent_set_callback(sent_dev_t *dev, uint8_t sdrv_sent_chanid, sent_recv_cb callback, void *arg);

/**
 * @brief sent callback handle function
 *
 * @param[in] dev sent dev
 * @param[in] sent_chan_id sent channel id
 */
void sent_callback_handle(sent_dev_t *dev, uint8_t sent_chan_id);

/**
 * @brief get sent frame info
 *
 * @param[in] dev sent dev
 * @param[in] sent_chan_id sent channel id
 * @param[in] data user data ptr
 * @param[in] status status value in sent frame
 */
uint32_t sent_get_frame(sent_dev_t *dev, uint8_t sdrv_sent_chanid, uint8_t *data, uint8_t *status);


#endif /* SENT_H */
