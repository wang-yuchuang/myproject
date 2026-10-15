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
#include <debug.h>
#include <types.h>
#include <cmsis_os2.h>

#include "driver/irq.h"
#include "driver/sent.h"
#include "crc4.h"

/**
 * @brief sent init
 *
 * @param[in] dev sent dev
 */
void sent_init(sent_dev_t *dev)
{
    ASSERT(dev != NULL);
    if(NULL != dev->mutex)
    {
        osMutexDelete(dev->mutex);
    }
    dev->mutex = osMutexNew(NULL);

    if(dev->ops->init != NULL)
    {
        for(uint8_t chan_id = 0; chan_id < dev->chan_num; ++chan_id)
        {
            if(true == dev->chan[chan_id].used)
            {
                dev->ops->init(dev, chan_id);
                dev->chan[chan_id].chan_sta = SENT_STATUS_INITIALIZED;
            }
        }
        dev->dev_sta = SENT_STATUS_INITED;
    }
}

/**
 * @brief sent deinit
 *
 * @param[in] dev sent dev
 */
void sent_deinit(sent_dev_t *dev)
{
    ASSERT(dev != NULL);

    if (dev->ops->deinit)
    {
        if(SENT_STATUS_INITED == dev->dev_sta)
        {
            for(uint8_t chan_id = 0; chan_id < dev->chan_num; ++chan_id)
            {
                if(SENT_STATUS_UNINITIALIZED != dev->chan[chan_id].chan_sta)
                {
                    dev->ops->deinit(dev, chan_id);
                    dev->chan[chan_id].chan_sta = SENT_STATUS_UNINITIALIZED;
                }
            }
            dev->dev_sta = SENT_STATUS_UNINIT;
        }
    }
    if(NULL != dev->mutex)
    {
        osMutexDelete(dev->mutex);
        dev->mutex = NULL;
    }
}

/**
 * @brief sent start recv frame
 *
 * @param[in] dev sent dev
 * @param[in] sent_chan_id sent channel id
 */
void sent_recv_start(sent_dev_t *dev, uint8_t sent_chan_id)
{
    ASSERT(dev != NULL);
    sent_chan_t *sent_chan = &dev->chan[sent_chan_id];
    osMutexAcquire(sent_chan->mutex, osWaitForever);

    if(sent_chan->chan_sta != SENT_STATUS_UNINITIALIZED)
    {
        if (sent_chan->chan_sta == SENT_STATUS_RUNNING)
        {
            dev->ops->recv_stop(dev, sent_chan_id);
        }

        /* set channel initial frame state */
        sent_chan->cur_state = SENT_IDLE;
        sent_chan->chan_sta = SENT_STATUS_RUNNING;
        dev->ops->recv_start(dev, sent_chan_id);

    }
    osMutexRelease(sent_chan->mutex);
}

/**
 * @brief sent stop recv frame
 *
 * @param[in] dev sent dev
 * @param[in] sent_chan_id sent channel id
 */
void sent_recv_stop(sent_dev_t *dev, uint8_t sent_chan_id)
{
    ASSERT(dev != NULL);
    sent_chan_t *sent_chan = &dev->chan[sent_chan_id];
    osMutexAcquire(sent_chan->mutex, osWaitForever);

    if (sent_chan->chan_sta == SENT_STATUS_RUNNING)
    {
        dev->ops->recv_stop(dev, sent_chan_id);
        /* clear channel initial frame state */
        sent_chan->cur_state = SENT_IDLE;
        sent_chan->chan_sta = SENT_STATUS_STOPED;
    }

    osMutexRelease(sent_chan->mutex);
}

/**
 * @brief parse sent frame
 *
 * @param[in] dev sent dev
 * @param[in] sent_chan_id sent channel id
 */
void sent_parse_msg(sent_dev_t *dev, uint8_t sent_chan_id)
{
    uint32_t reminder = 0;
    sent_chan_t *sent_chan = &dev->chan[sent_chan_id];

    /* read sent frame value */
    uint32_t fifo_datanum = dev->ops->get_datanum(dev, sent_chan_id);
    uint32_t fifo_data = 0;

    for(int i = 0; i < fifo_datanum; ++i){
        fifo_data = dev->ops->get_data(dev, sent_chan_id);
        switch(sent_chan->cur_state)
        {
            case SENT_IDLE:
            {
                /* Verification frequency valid */
                uint32_t scaler = fifo_data/56;
                uint32_t err_range = (scaler * 10) / (sent_chan->expect_scaler);
                sent_chan->frame_err = SENT_OK;
                if((err_range >= 8) && (err_range <= 12))
                {
                    sent_chan->actual_scaler = scaler;
                    sent_chan->cur_state = SENT_SYNC;
                }
                break;
            }
            case SENT_SYNC:
            {
                uint32_t status_tick = (fifo_data * 10) / sent_chan->actual_scaler;
                reminder = (status_tick % 100) % 10;
                status_tick = (reminder < 5) ? (status_tick / 10) : (status_tick / 10 + 1);

                /* Verification tick number */
                if ((status_tick < 12) || (status_tick > 27)) {
                    sent_chan->frame_err |= SENT_STATUS_ERR;
                }
                /* status value */
                sent_chan->frame_sta = (status_tick - 12);
                sent_chan->rx_index = 0;
                sent_chan->cur_state = SENT_DATA;
                break;
            }
            case SENT_DATA:
            {
                uint32_t data_tick = (fifo_data * 10) / sent_chan->actual_scaler;
                reminder = (data_tick % 100) % 10;
                data_tick = (reminder < 5) ? (data_tick / 10) : (data_tick / 10 + 1);
                uint8_t rx_index = sent_chan->rx_index;
                if ((data_tick < 12) || (data_tick > 27)) {
                    sent_chan->frame_err |= SENT_DATA_ERR;
                }
                /* Data Value */
                sent_chan->rxframe[rx_index] = (data_tick - 12);
                sent_chan->rx_index += 1;
                if(sent_chan->rx_index == sent_chan->frame_dl)
                {
                    sent_chan->rx_index = 0;
                    sent_chan->cur_state = SENT_CRC;
                }
                break;
            }
            case SENT_CRC:
            {
                uint32_t crc_tick = (fifo_data * 10) / sent_chan->actual_scaler;
                reminder = (crc_tick % 100) % 10;
                crc_tick = (reminder < 5) ? (crc_tick / 10) : (crc_tick / 10 + 1);
                if ((crc_tick < 12) || (crc_tick > 27)) {
                    sent_chan->frame_err |= SENT_CRC_ERR;
                }
                uint8_t data_len = sent_chan->frame_dl;
                uint8_t crc_calc = crc4_calculate(5, sent_chan->rxframe, data_len);
                if (crc_calc != (crc_tick - 12)) {
                    sent_chan->frame_err |= SENT_CRC_ERR;
                }
                /* crc value */
               sent_chan->frame_crc = crc_tick - 12;
               if(sent_chan->pause_pulse == true)
               {
                    sent_chan->cur_state = SENT_PAUSE;
               }
               else
               {
                    sent_chan->cur_state = SENT_IDLE;
                    sent_callback_handle(dev, sent_chan_id);
               }
               break;
            }
            case SENT_PAUSE:
            {
                sent_chan->cur_state = SENT_IDLE;
                sent_callback_handle(dev, sent_chan_id);
                break;
            }
            default:
                break;
        }
    }

}

/**
 * @brief sent set callback function
 *
 * @param[in] dev sent dev
 * @param[in] sent_chan_id sent channel id
 * @param[in] callback callback function
 * @param[in] arg callback arg
 */
void sent_set_callback(sent_dev_t *dev, uint8_t sdrv_sent_chanid, sent_recv_cb callback, void *arg)
{
    ASSERT(dev != NULL);

    sent_chan_t *sent_chan = &dev->chan[sdrv_sent_chanid];
    sent_chan->frameover_cb = callback;
    sent_chan->arg = arg;
}

/**
 * @brief sent callback handle function
 *
 * @param[in] dev sent dev
 * @param[in] sent_chan_id sent channel id
 */
void sent_callback_handle(sent_dev_t *dev, uint8_t sent_chan_id)
{
    ASSERT(dev != NULL);

    sent_chan_t *sent_chan = &dev->chan[sent_chan_id];

    if(sent_chan->frameover_cb)
    {
        sent_chan->frameover_cb(sent_chan->arg);
    }
}

/**
 * @brief get sent frame info
 *
 * @param[in] dev sent dev
 * @param[in] sent_chan_id sent channel id
 * @param[in] data user data ptr
 * @param[in] status status value in sent frame
 */
uint32_t sent_get_frame(sent_dev_t *dev, uint8_t sdrv_sent_chanid, uint8_t *data, uint8_t *status)
{
    sent_chan_t *sent_chan = &dev->chan[sdrv_sent_chanid];
    uint32_t ret_val = (sent_err_t)SENT_NOT_OK;

    /* check the channel status */
    if (sent_chan->cur_state != SENT_IDLE)
    {
        return ret_val;
    }

    for(uint8_t i = 0; i < 6; ++i)
    {
        data[i] = sent_chan->rxframe[i];
        sent_chan->rxframe[i] = 0;
    }
    *status = sent_chan->frame_sta;
    ret_val = sent_chan->frame_err;

    return ret_val;
}

