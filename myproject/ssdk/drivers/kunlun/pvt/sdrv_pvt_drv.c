/**
 * @file sdrv_pvt_drv.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: pvt driver implement file
 *
 * Revision History:
 * -----------------
*/
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <cmsis_os2.h>
#include <types.h>
#include <spinlock.h>
#include <driver/irq.h>
#include <board.h>

#include <sdrv_pvt.h>
#include <driver/kunlun/pvt/sdrv_pvt_drv.h>

#define SDRV_PVT_DEBUG_LVL      SSDK_WARNING

#define EXTRA_FMT   "sdrv_pvt:"

#if SDRV_PVT_DEBUG_LVL >= SSDK_ERR
#define SDRV_PVT_ERR(format, ...) \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDRV_PVT_ERR(format, ...)
#endif

#if SDRV_PVT_DEBUG_LVL >= SSDK_WARNING
#define SDRV_PVT_WARN(format, ...) \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDRV_PVT_WARN(format, ...)
#endif

#if SDRV_PVT_DEBUG_LVL >= SSDK_INFO
#define SDRV_PVT_INFO(format, ...) \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDRV_PVT_INFO(format, ...)
#endif

#define SENSOR_REF_CLK  1200000     /**< 1.2M */

/** function declare section */
static int32_t sdrv_pvt_init(pvt_dev_t* dev);
static int32_t sdrv_pvt_deinit(pvt_dev_t* dev);
static int32_t sdrv_pvt_set_alarm(pvt_dev_t* dev
                                    ,pvt_alarm_mode_enum_t mode
                                    ,pvt_alarm_type_enum_t type
                                    ,const pvt_alarm_para_t* para);
static int32_t sdrv_pvt_clr_alarm(pvt_dev_t* dev, int32_t chanId);
static int32_t sdrv_pvt_enable_alarm(pvt_dev_t* dev, int32_t chanId);
static int32_t sdrv_pvt_disable_alarm(pvt_dev_t* dev, int32_t chanId);
static int32_t sdrv_pvt_get_pvt(pvt_dev_t* dev, pvt_data_t* pvtData);
static int32_t sdrv_pvt_integer_data_get(pvt_dev_t* dev, pvt_integer_get_enum_t type, int32_t chanId, int32_t* data);
static int32_t sdrv_pvt_float_data_get(pvt_dev_t* dev, pvt_float_get_enum_t type, int32_t chanId, float* data);
static int32_t sdrv_pvt_integer_data_set(pvt_dev_t* dev, pvt_integer_set_enum_t type, int32_t data);

static int32_t channel_enable(sdrv_pvt_chan_t* chan, bool isEnable);


/** function implement section */
static void clear_byte_regions(uint8_t* data, uint8_t startBit, uint8_t bitLen)
{
    // check parameter
    if ((startBit + bitLen) >= (sizeof(char) * 8)) {
        return;
    }

    // clear bit region
    uint8_t mask_win = 0;
    mask_win = (1 << bitLen) - 1;
    mask_win = mask_win << startBit;
    mask_win = ~mask_win;

    *data &= mask_win;
}

static int32_t convert_alarmType2lld(pvt_alarm_type_enum_t type)
{
    int32_t ret = -1;

    switch(type) {
        case PVT_ALARM_TYPE_HYST_HIGH:
            ret = PVT_SENSOR_ALARM_TYPE_HYST_H;
            break;
        case PVT_ALARM_TYPE_HYST_LOW:
            ret = PVT_SENSOR_ALARM_TYPE_HYST_L;
            break;
        case PVT_ALARM_TYPE_HYST_RAISING:
            ret = PVT_SENSOR_ALARM_TYPE_HYST_R;
            break;
        case PVT_ALARM_TYPE_HYST_FALLING:
            ret = PVT_SENSOR_ALARM_TYPE_HYST_F;
            break;
        default:
            break;
    }

    return ret;
}

static int32_t alarm_handle(uint32_t irq, void* usrData)
{
    sdrv_pvt_chan_t* chan_ptr = (sdrv_pvt_chan_t *)usrData;
    // disable int
    channel_enable(chan_ptr, false);

    if (chan_ptr->alarm_type == PVT_ALARM_TYPE_HYST_FALLING) {
        sdrv_pvt_lld_get_hyst_timer_f(chan_ptr->dev_ptr->base, chan_ptr->chan, &chan_ptr->sustain_ftime);
    }
    else if (chan_ptr->alarm_type == PVT_ALARM_TYPE_HYST_RAISING) {
        sdrv_pvt_lld_get_hyst_timer_r(chan_ptr->dev_ptr->base, chan_ptr->chan, &chan_ptr->sustain_rtime);
    }
    else {
        // SDRV_PVT_ERR("current channel alarm type: %d do not support get sustain time\n", chan_ptr->alarm_type);
    }

    // call function
    if (chan_ptr->alarm_cb != NULL) {
        chan_ptr->alarm_cb(chan_ptr->chanId, chan_ptr->usrData);
    }

    return 0;
}

static uint16_t convert_phyVal2Code(sdrv_sensor_conv_cfg_t* cfg, float phyVal)
{
    float base = 1.0;
    float code = 0.0;
    for (int32_t i = 0; i < cfg->par_num; i++) {
        code += base * cfg->para_arry[i];
        base *= phyVal;
    }

    return (uint16_t)(code);
}

static float convert_code2PhyVal(sdrv_sensor_conv_cfg_t* cfg, uint32_t code)
{
    float base = 1.0;
    float phy_val = 0.0;
    for (int32_t i = 0; i < cfg->par_num; i++) {
        phy_val += base * cfg->para_arry[i];
        base *= code;
    }

    return phy_val;
}

static int32_t channel_enable(sdrv_pvt_chan_t* chan, bool isEnable)
{
    if (chan == NULL || chan->chanId == -1) {
        SDRV_PVT_ERR("parameter is error or channel is invalid for %s \n", __FUNCTION__);
        return -1;
    }

    if (isEnable) {
        sdrv_pvt_lld_set_int(chan->dev_ptr->base
                                    , chan->chan
                                    , convert_alarmType2lld(chan->alarm_type)
                                    , PVT_REG_ENABLE);
        // irq_enable(chan->irq);
    }
    else {
        sdrv_pvt_lld_set_int(chan->dev_ptr->base
                                    , chan->chan
                                    , convert_alarmType2lld(chan->alarm_type)
                                    , PVT_REG_DISABLE);
        // irq_disable(chan->irq);
    }

    return 0;
}


static int32_t channel_config(sdrv_pvt_chan_t* chan, bool isSet)
{
    if (chan == NULL || chan->chanId == -1) {
        SDRV_PVT_ERR("parameter is error or channel is invalid for %s \n", __FUNCTION__);
        return -1;
    }

    if (isSet) {
        // diable alarm channel first
        channel_enable(chan, false);

        // set ctrl disable
        sdrv_pvt_lld_set_ctrl_enable(chan->dev_ptr->base, PVT_REG_DISABLE);

        // set ctrl alarm mode
        sdrv_pvt_lld_set_ctrl_alarm_mode(chan->dev_ptr->base
                                            , chan->chan
                                            , convert_alarmType2lld(chan->alarm_type)
                                            , PVT_REG_ENABLE);
        // set hyst data
        switch (chan->alarm_type) {
            case PVT_ALARM_TYPE_HYST_HIGH:
                sdrv_pvt_lld_set_hyst_high(chan->dev_ptr->base
                                            , chan->chan
                                            , chan->high_code
                                            , chan->low_code);
                break;
            case PVT_ALARM_TYPE_HYST_LOW:
                sdrv_pvt_lld_set_hyst_low(chan->dev_ptr->base
                                            , chan->chan
                                            , chan->high_code
                                            , chan->low_code);
                break;
            case PVT_ALARM_TYPE_HYST_RAISING:
                sdrv_pvt_lld_set_hyst_raising(chan->dev_ptr->base
                                            , chan->chan
                                            , chan->high_code
                                            , chan->low_code);
                break;
            case PVT_ALARM_TYPE_HYST_FALLING:
                sdrv_pvt_lld_set_hyst_falling(chan->dev_ptr->base
                                            , chan->chan
                                            , chan->high_code
                                            , chan->low_code);
                break;
            default:
                break;
        }

        // set ctrl enable
        sdrv_pvt_lld_set_ctrl_enable(chan->dev_ptr->base, PVT_REG_ENABLE);

        // attach irq handle
        irq_disable(chan->irq);
        irq_attach(chan->irq, alarm_handle, chan);
        irq_enable(chan->irq);
    }
    else {
        irq_disable(chan->irq);
        irq_detach(chan->irq);

        // set ctrl alarm mode
        sdrv_pvt_lld_set_ctrl_alarm_mode(chan->dev_ptr->base
                                        , chan->chan
                                        , convert_alarmType2lld(chan->alarm_type)
                                        , PVT_REG_DISABLE);
        // disable channel id
        chan->chanId = -1;
    }

    return 0;
}


static sdrv_sensor_conv_cfg_t* get_phyVal2Code_cfg(sdrv_pvt_config_t* cfgPtr, pvt_alarm_mode_enum_t alarmMode)
{
    if (cfgPtr == NULL) {
        return NULL;
    }

    if (alarmMode == PVT_ALARM_MODE_TEMP) {
        return &cfgPtr->conv_temp2code;
    }
    else if (alarmMode == PVT_ALARM_MODE_VOLTAGE) {
        return &cfgPtr->conv_vol2code;
    }
    else {
        return NULL;
    }
}

static sdrv_sensor_conv_cfg_t* get_code2PhyVal_cfg(sdrv_pvt_config_t* cfgPtr, pvt_alarm_mode_enum_t alarmMode)
{
    if (cfgPtr == NULL) {
        return NULL;
    }

    if (alarmMode == PVT_ALARM_MODE_TEMP) {
        return &cfgPtr->conv_code2temp;
    }
    else if (alarmMode == PVT_ALARM_MODE_VOLTAGE) {
        return &cfgPtr->conv_code2vol;
    }
    else {
        return NULL;
    }
}

sdrv_pvt_chan_t* chan_match(pvt_dev_t* dev, int32_t chanId)
{
    if (dev == NULL ||
        chanId < 0) {
        SDRV_PVT_ERR("parameter error for %s\n", __FUNCTION__);
        return NULL;
    }

    sdrv_pvt_dev_t* sdrv_dev_ptr = dev->priv;
    for (int32_t i = 0; i < PVT_SENSOR_ALARM_CHANNUM; i++) {
        if (sdrv_dev_ptr->chan[i].chanId == chanId) {
            return &sdrv_dev_ptr->chan[i];
        }
    }

    return NULL;
}

static int32_t wait_data_valid(pvt_dev_t* dev)
{
    sdrv_pvt_dev_t* sdrv_dev_ptr = dev->priv;
    sdrv_pvt_config_t* cfg_ptr = sdrv_dev_ptr->cfg_ptr;
    int32_t data_valid = 0;
    int32_t retry_cnt = 0;

    if (cfg_ptr->conversion_time == 0) {
        sdrv_pvt_lld_get_dout_valid(dev->base, &data_valid);
        while (data_valid == 0 && retry_cnt++ < 5) {
            udelay(100);
            sdrv_pvt_lld_get_dout_valid(dev->base, &data_valid);
        }
    }
    else {
        udelay(cfg_ptr->conversion_time);
    }

    return 0;
}

static int32_t set_sensor_sample_mode(pvt_dev_t* dev, uint8_t mode)
{
    if (dev == NULL ||
        mode >= PVT_SENSOR_SAMPLE_MODE_INVALID) {
        SDRV_PVT_ERR("invalid parameter for %s\n", __FUNCTION__);
        return -1;
    }
    sdrv_pvt_dev_t* sdrv_dev_ptr = dev->priv;

    sdrv_pvt_lld_set_ctrl_enable(dev->base, PVT_REG_DISABLE);
    sdrv_pvt_lld_set_ctrl_sample_mode(dev->base, sdrv_dev_ptr->sensor_type, mode);
    sdrv_pvt_lld_set_ctrl_enable(dev->base, PVT_REG_ENABLE);
    wait_data_valid(dev);

    return 0;
}

static pvt_ops_t s_sdrv_pvt_ops = {
    .init = sdrv_pvt_init,
    .deinit = sdrv_pvt_deinit,
    .disable_alarm = sdrv_pvt_disable_alarm,
    .enable_alarm = sdrv_pvt_enable_alarm,
    .set_alarm = sdrv_pvt_set_alarm,
    .clr_alarm = sdrv_pvt_clr_alarm,
    .get_pvt = sdrv_pvt_get_pvt,
    .integer_data_get = sdrv_pvt_integer_data_get,
    .integer_data_set = sdrv_pvt_integer_data_set,
    .float_data_get = sdrv_pvt_float_data_get,
};

/**
 * @brief init
 *
 */
int32_t sdrv_pvt_init(pvt_dev_t* dev)
{

    if (dev == NULL ||
        dev->priv == NULL) {
        SDRV_PVT_ERR("invalid sdrv dev for %s\n", __FUNCTION__);
        return -1;
    }

    sdrv_pvt_dev_t* sdrv_dev_ptr = dev->priv;
    sdrv_pvt_config_t* sdrv_cfg_ptr = sdrv_dev_ptr->cfg_ptr;

    // lock init
    spin_lock_init(&sdrv_dev_ptr->lock);
    sdrv_dev_ptr->chan_bit = 0;
    sdrv_dev_ptr->gen_id = 0;
    sdrv_dev_ptr->sensor_mode = sdrv_cfg_ptr->sensor_mode;
    sdrv_dev_ptr->sensor_type = sdrv_cfg_ptr->sensor_type;

    // chan init
    sdrv_dev_ptr->chan_bit = 0;
    for (int32_t i = 0; i < PVT_SENSOR_ALARM_CHANNUM; i++) {
        sdrv_dev_ptr->chan[i].chan = i;
        sdrv_dev_ptr->chan[i].chanId = -1;
        sdrv_dev_ptr->chan[i].sustain_rtime = 0;
        sdrv_dev_ptr->chan[i].sustain_ftime = 0;
        sdrv_dev_ptr->chan[i].dev_ptr = dev;
        sdrv_dev_ptr->chan[i].irq = sdrv_cfg_ptr->irq_num[i];
        sdrv_dev_ptr->chan[i].alarm_mode = PVT_ALARM_MODE_INVALID;
        sdrv_dev_ptr->chan[i].alarm_cb = NULL;
        sdrv_dev_ptr->chan[i].usrData = NULL;
    }

    // check config
    if (sdrv_cfg_ptr->conv_code2temp.par_num > PVT_SENSOR_CONVERT_PARA_MAX ||
        sdrv_cfg_ptr->conv_code2vol.par_num > PVT_SENSOR_CONVERT_PARA_MAX ||
        sdrv_cfg_ptr->conv_temp2code.par_num > PVT_SENSOR_CONVERT_PARA_MAX ||
        sdrv_cfg_ptr->conv_vol2code.par_num > PVT_SENSOR_CONVERT_PARA_MAX) {
        SDRV_PVT_ERR("invalid config data for %s\n", __FUNCTION__);
        return -1;
    }

    // set mode and init
    sdrv_pvt_lld_set_ctrl_ctlMode(dev->base, 1);

    sdrv_pvt_lld_set_ctrl_sample_mode(dev->base
                                        , sdrv_cfg_ptr->sensor_type
                                        , sdrv_cfg_ptr->sensor_mode);

    sdrv_pvt_lld_set_ctrl_enable(dev->base, PVT_REG_ENABLE);

    return 0;
}

int32_t sdrv_pvt_deinit(pvt_dev_t* dev)
{
    if (dev == NULL ||
        dev->priv == NULL) {
        SDRV_PVT_ERR("parameter error for %s\n", __FUNCTION__);
        return -1;
    }

    irq_state_t state;
    sdrv_pvt_dev_t* sdrv_dev_ptr = dev->priv;

    state = spin_lock_irqsave(&sdrv_dev_ptr->lock);

    for (int32_t i = 0; i < PVT_SENSOR_ALARM_CHANNUM; i++) {
        if (sdrv_dev_ptr->chan[i].chanId != -1) {
            // disable channel
            channel_enable(&sdrv_dev_ptr->chan[i], false);
            // clean channel
            channel_config(&sdrv_dev_ptr->chan[i], false);
        }

    }

    spin_unlock_irqrestore(&sdrv_dev_ptr->lock, state);
    return 0;
}


int32_t sdrv_pvt_set_alarm(pvt_dev_t* dev
                            ,pvt_alarm_mode_enum_t mode
                            ,pvt_alarm_type_enum_t type
                            ,const pvt_alarm_para_t* para)
{
    if (dev == NULL ||
        dev->priv == NULL ||
        para == NULL) {
        SDRV_PVT_ERR("parameter error for %s\n", __FUNCTION__);
        return -1;
    }

    irq_state_t state;
    sdrv_pvt_dev_t* sdrv_dev_ptr = dev->priv;
    sdrv_sensor_conv_cfg_t* conv_cfg_ptr = NULL;
    int32_t ret = -1;

    state = spin_lock_irqsave(&sdrv_dev_ptr->lock);

    // check whether there are free channel
    if (sdrv_dev_ptr->chan_bit == ((1 << PVT_SENSOR_ALARM_CHANNUM) - 1)) {
        SDRV_PVT_ERR("there is not valid alarm channel for %s\n", __FUNCTION__);
        goto errOut;
    }

    // check whether the current mode meets requirements
    if (sdrv_dev_ptr->sensor_mode != mode) {
        SDRV_PVT_ERR("current sensor mode: %d is not matching current alarm mode: %d"
                        , sdrv_dev_ptr->sensor_mode
                        , mode);
        goto errOut;
    }

    // check config ptr is valid
    if ((conv_cfg_ptr = get_phyVal2Code_cfg(sdrv_dev_ptr->cfg_ptr, mode)) == NULL) {
        SDRV_PVT_ERR("get convert config error for %s\n", __FUNCTION__);
        goto errOut;
    }

    for (int32_t i = 0; i < PVT_SENSOR_ALARM_CHANNUM; i++) {
        // valid channel
        if (sdrv_dev_ptr->chan[i].chanId == -1) {
            sdrv_dev_ptr->chan[i].chanId = sdrv_dev_ptr->gen_id++;
            sdrv_dev_ptr->chan[i].alarm_mode = mode;
            sdrv_dev_ptr->chan[i].alarm_type = type;
            sdrv_dev_ptr->chan[i].high_code = convert_phyVal2Code(conv_cfg_ptr, para->high_val);
            sdrv_dev_ptr->chan[i].low_code = convert_phyVal2Code(conv_cfg_ptr, para->low_val);
            sdrv_dev_ptr->chan[i].alarm_cb = para->cb_fun;
            sdrv_dev_ptr->chan[i].usrData = para->args;

            // update chan bit
            sdrv_dev_ptr->chan_bit |= 1 << i;

            // config chan
            channel_config(&sdrv_dev_ptr->chan[i], true);

            ret = sdrv_dev_ptr->chan[i].chanId;

            // enable channel
            channel_enable(&sdrv_dev_ptr->chan[i], true);

            break;
        }
    }

    spin_unlock_irqrestore(&sdrv_dev_ptr->lock, state);
    return ret;

errOut:
    spin_unlock_irqrestore(&sdrv_dev_ptr->lock, state);
    return -1;
}



static int32_t sdrv_pvt_clr_alarm(pvt_dev_t* dev, int32_t chanId)
{
    if (dev == NULL ||
        chanId < 0) {
        SDRV_PVT_ERR("parameter error for %s\n", __FUNCTION__);
        return -1;
    }

    irq_state_t state;
    bool is_valid = false;
    sdrv_pvt_dev_t* sdrv_dev_ptr = dev->priv;
    sdrv_pvt_chan_t* chan_ptr = NULL;

    state = spin_lock_irqsave(&sdrv_dev_ptr->lock);

    if ((chan_ptr = chan_match(dev, chanId)) != NULL) {
        // disable channel and clear it
        channel_enable(chan_ptr, false);
        channel_config(chan_ptr, false);

        // clean chan bit
        clear_byte_regions(&sdrv_dev_ptr->chan_bit, chan_ptr->chan, 1);

        is_valid = true;

    }

    spin_unlock_irqrestore(&sdrv_dev_ptr->lock, state);

    if (!is_valid) {
        SDRV_PVT_ERR("channel id: %d is invalid for %s\n", chanId, __FUNCTION__);
        return -1;
    }
    return 0;
}

static int32_t sdrv_pvt_enable_alarm(pvt_dev_t* dev, int32_t chanId)
{
    if (dev == NULL ||
        chanId < 0) {
        SDRV_PVT_ERR("parameter error for %s\n", __FUNCTION__);
        return -1;
    }

    irq_state_t state;
    bool is_valid = false;
    sdrv_pvt_dev_t* sdrv_dev_ptr = dev->priv;
    sdrv_pvt_chan_t* chan_ptr = NULL;

    state = spin_lock_irqsave(&sdrv_dev_ptr->lock);

    if ((chan_ptr = chan_match(dev, chanId)) != NULL) {
        // clr int
        sdrv_pvt_lld_clr_int_sta(chan_ptr->dev_ptr->base
                                , chan_ptr->chan
                                , convert_alarmType2lld(chan_ptr->alarm_type));
        channel_enable(chan_ptr, true);
        is_valid = true;
    }

    spin_unlock_irqrestore(&sdrv_dev_ptr->lock, state);

    if (!is_valid) {
        SDRV_PVT_ERR("channel id: %d is invalid for %s\n", chanId, __FUNCTION__);
        return -1;
    }
    return 0;
}

static int32_t sdrv_pvt_disable_alarm(pvt_dev_t* dev, int32_t chanId)
{
    if (dev == NULL ||
        chanId < 0) {
        SDRV_PVT_ERR("parameter error for %s\n", __FUNCTION__);
        return -1;
    }

    irq_state_t state;
    bool is_valid = false;
    sdrv_pvt_dev_t* sdrv_dev_ptr = dev->priv;
    sdrv_pvt_chan_t* chan_ptr = NULL;

    state = spin_lock_irqsave(&sdrv_dev_ptr->lock);

    if ((chan_ptr = chan_match(dev, chanId)) != NULL) {
        channel_enable(chan_ptr, false);
        is_valid = true;
    }

    spin_unlock_irqrestore(&sdrv_dev_ptr->lock, state);

    if (!is_valid) {
        SDRV_PVT_ERR("channel id: %d is invalid for %s\n", chanId, __FUNCTION__);
        return -1;
    }
    return 0;
}


static int32_t sdrv_pvt_get_pvt(pvt_dev_t* dev, pvt_data_t* pvtData)
{
    if (dev == NULL ||
        pvtData == NULL) {
        SDRV_PVT_ERR("parameter error for %s\n", __FUNCTION__);
        return -1;
    }

    sdrv_pvt_dev_t* sdrv_dev_ptr = dev->priv;
    sdrv_pvt_config_t* cfg_ptr = sdrv_dev_ptr->cfg_ptr;
    irq_state_t state;
    int32_t data_out = 0;

    if (sdrv_dev_ptr->chan_bit != 0) {
        SDRV_PVT_ERR("please clean all alarm channel(0x%x) or reset device before get pvt data\n", sdrv_dev_ptr->chan_bit);
        return -1;
    }

    ASSERT(cfg_ptr->sensor_clk != 0);

    state = spin_lock_irqsave(&sdrv_dev_ptr->lock);

    memset(pvtData, 0x00, sizeof(pvt_data_t));
    // set p mode
    set_sensor_sample_mode(dev, PVT_SENSOR_SAMPLE_MODE_PROCESS);
    sdrv_pvt_lld_get_dout_data(dev->base, &data_out);
    pvtData->process = data_out * (SENSOR_REF_CLK / cfg_ptr->sensor_clk);

    // set t mode
    set_sensor_sample_mode(dev, PVT_SENSOR_SAMPLE_MODE_TEMP);
    sdrv_pvt_lld_get_dout_data(dev->base, &data_out);
    pvtData->temp = convert_code2PhyVal(&cfg_ptr->conv_code2temp, data_out);

    // set v mode
    set_sensor_sample_mode(dev, PVT_SENSOR_SAMPLE_MODE_VOLTAGE);
    sdrv_pvt_lld_get_dout_data(dev->base, &data_out);
    pvtData->voltage = convert_code2PhyVal(&cfg_ptr->conv_code2vol, data_out);

    // set to default mode
    set_sensor_sample_mode(dev, sdrv_dev_ptr->sensor_mode);

    spin_unlock_irqrestore(&sdrv_dev_ptr->lock, state);

    return 0;
}

int32_t get_integer_data_sensor_sample_mode(pvt_dev_t* dev, int32_t* data)
{
    sdrv_pvt_dev_t* sdrv_dev_ptr = dev->priv;

    *data = sdrv_dev_ptr->sensor_mode;

    return 0;
}

int32_t get_integer_data_process(pvt_dev_t* dev, int32_t* data)
{
    sdrv_pvt_dev_t* sdrv_dev_ptr = dev->priv;
    sdrv_pvt_config_t* cfg_ptr = sdrv_dev_ptr->cfg_ptr;
    int32_t data_out = 0;

    if (sdrv_dev_ptr->sensor_mode != PVT_SENSOR_SAMPLE_MODE_PROCESS &&
        sdrv_dev_ptr->chan_bit != 0) {
        SDRV_PVT_ERR("please clean all alarm channel(0x%x) or reset device for %s\n", sdrv_dev_ptr->chan_bit, __FUNCTION__);
        return -1;
    }

    set_sensor_sample_mode(dev, PVT_SENSOR_SAMPLE_MODE_PROCESS);
    sdrv_pvt_lld_get_dout_data(dev->base, &data_out);
    *data = data_out * (SENSOR_REF_CLK / cfg_ptr->sensor_clk);

    if (sdrv_dev_ptr->sensor_mode != PVT_SENSOR_SAMPLE_MODE_PROCESS) {
        set_sensor_sample_mode(dev, sdrv_dev_ptr->sensor_mode);
    }

    return 0;
}


int32_t get_integer_data_alarm_sustain_time(pvt_dev_t* dev, int32_t chanId, int32_t* data)
{
    sdrv_pvt_chan_t* chan_ptr = NULL;

    if ((chan_ptr = chan_match(dev, chanId)) != NULL) {
        if (chan_ptr->alarm_type == PVT_ALARM_TYPE_HYST_FALLING) {
            // sdrv_pvt_lld_get_hyst_timer_f(dev->base, chan_ptr->chan, &time);
            // *data = (int32_t)time;
            *data = chan_ptr->sustain_ftime;
        }
        else if (chan_ptr->alarm_type == PVT_ALARM_TYPE_HYST_RAISING) {
            // sdrv_pvt_lld_get_hyst_timer_r(dev->base, chan_ptr->chan, &time);
            // *data = (int32_t)time;
            *data = chan_ptr->sustain_rtime;
        }
        else {
            SDRV_PVT_ERR("current channel alarm type: %d do not support get sustain time\n", chan_ptr->alarm_type);
        }
    }
    else {
        SDRV_PVT_ERR("invlaid chanId: %d for %s.\n", chanId, __FUNCTION__);
    }

    return 0;
}

int32_t get_integer_data_alarm_type(pvt_dev_t* dev, int32_t chanId, int32_t* data)
{
    sdrv_pvt_chan_t* chan_ptr = NULL;

    if ((chan_ptr = chan_match(dev, chanId)) != NULL) {
        *data = chan_ptr->alarm_type;
    }
    else {
        SDRV_PVT_ERR("invlaid chanId: %d for %s.\n", chanId, __FUNCTION__);
    }
    return 0;
}

int32_t get_integer_data_alarm_status(pvt_dev_t* dev, int32_t chanId, int32_t* data)
{
    sdrv_pvt_chan_t* chan_ptr = NULL;

    if ((chan_ptr = chan_match(dev, chanId)) != NULL) {
        sdrv_pvt_lld_get_int_status(dev->base
                                    , chan_ptr->chan
                                    , convert_alarmType2lld(chan_ptr->alarm_type)
                                    , data);
    }
    else {
        SDRV_PVT_ERR("invlaid chanId: %d for %s.\n", chanId, __FUNCTION__);
    }

    return 0;
}

int32_t get_integer_data_alarm_enable_status(pvt_dev_t* dev, int32_t chanId, int32_t* data)
{
    sdrv_pvt_chan_t* chan_ptr = NULL;

    if ((chan_ptr = chan_match(dev, chanId)) != NULL) {
        sdrv_pvt_lld_get_int(dev->base
                             , chan_ptr->chan
                             , convert_alarmType2lld(chan_ptr->alarm_type)
                             , data);
    }
    else {
        SDRV_PVT_ERR("invlaid chanId: %d for %s.\n", chanId, __FUNCTION__);
    }

    return 0;
}

static int32_t sdrv_pvt_integer_data_get(pvt_dev_t* dev, pvt_integer_get_enum_t type, int32_t chanId, int32_t* data)
{
    int32_t ret = 0;

    switch (type) {
        case PVT_GET_INT_PROCESS:
            ret = get_integer_data_process(dev, data);
            break;
        case PVT_GET_INT_SENSOR_SAMPLE_MODE:
            ret = get_integer_data_sensor_sample_mode(dev, data);
            break;
        case PVT_GET_INT_ALARM_SUSTAIN_TIME:
            ret = get_integer_data_alarm_sustain_time(dev, chanId, data);
            break;
        case PVT_GET_INT_ALARM_TYPE:
            ret = get_integer_data_alarm_type(dev, chanId, data);
            break;
        case PVT_GET_INT_ALARM_STATUS:
            ret = get_integer_data_alarm_status(dev, chanId, data);
            break;
        case PVT_GET_INT_ALARM_ENABLE_STA:
            ret = get_integer_data_alarm_enable_status(dev, chanId, data);
            break;
        default:
            SDRV_PVT_ERR("invalid get data type: %d for %s\n", type, __FUNCTION__);
            break;
    }

    return ret;
}

static int32_t get_float_data_cur_temp(pvt_dev_t* dev, float* data)
{
    sdrv_pvt_dev_t* sdrv_dev_ptr = dev->priv;
    sdrv_pvt_config_t* cfg_ptr = sdrv_dev_ptr->cfg_ptr;
    int32_t data_out = 0;

    if (sdrv_dev_ptr->sensor_mode != PVT_SENSOR_SAMPLE_MODE_TEMP &&
        sdrv_dev_ptr->chan_bit != 0) {
        SDRV_PVT_ERR("please clean all alarm channel(0x%x) or reset device before get pvt data\n", sdrv_dev_ptr->chan_bit);
        return -1;
    }

    set_sensor_sample_mode(dev, PVT_SENSOR_SAMPLE_MODE_TEMP);
    sdrv_pvt_lld_get_dout_data(dev->base, &data_out);
    *data = convert_code2PhyVal(&cfg_ptr->conv_code2temp, data_out);

    if (sdrv_dev_ptr->sensor_mode != PVT_SENSOR_SAMPLE_MODE_TEMP) {
        set_sensor_sample_mode(dev, sdrv_dev_ptr->sensor_mode);
    }

    return 0;
}

static int32_t get_float_data_cur_voltage(pvt_dev_t* dev, float* data)
{
    sdrv_pvt_dev_t* sdrv_dev_ptr = dev->priv;
    sdrv_pvt_config_t* cfg_ptr = sdrv_dev_ptr->cfg_ptr;
    int32_t data_out = 0;

    if (sdrv_dev_ptr->sensor_mode != PVT_SENSOR_SAMPLE_MODE_VOLTAGE &&
        sdrv_dev_ptr->chan_bit != 0) {
        SDRV_PVT_ERR("please clean all alarm channel(0x%x) or reset device before get pvt data\n", sdrv_dev_ptr->chan_bit);
        return -1;
    }

    set_sensor_sample_mode(dev, PVT_SENSOR_SAMPLE_MODE_VOLTAGE);
    sdrv_pvt_lld_get_dout_data(dev->base, &data_out);
    *data = convert_code2PhyVal(&cfg_ptr->conv_code2vol, data_out);

    if (sdrv_dev_ptr->sensor_mode != PVT_SENSOR_SAMPLE_MODE_VOLTAGE) {
        set_sensor_sample_mode(dev, sdrv_dev_ptr->sensor_mode);
    }

    return 0;
}

static int32_t get_float_data_alarm_phyval_high(pvt_dev_t* dev, int32_t chanId, float* data)
{
    sdrv_pvt_chan_t* chan_ptr = NULL;
    sdrv_pvt_dev_t* sdrv_dev_ptr = dev->priv;
    sdrv_pvt_config_t* cfg_ptr = sdrv_dev_ptr->cfg_ptr;

    if ((chan_ptr = chan_match(dev, chanId)) != NULL) {
        *data = convert_code2PhyVal(get_code2PhyVal_cfg(cfg_ptr, chan_ptr->alarm_mode)
                    , chan_ptr->high_code);
    }

    return 0;
}

static int32_t get_float_data_alarm_phyval_low(pvt_dev_t* dev, int32_t chanId, float* data)
{
    sdrv_pvt_chan_t* chan_ptr = NULL;
    sdrv_pvt_dev_t* sdrv_dev_ptr = dev->priv;
    sdrv_pvt_config_t* cfg_ptr = sdrv_dev_ptr->cfg_ptr;

    if ((chan_ptr = chan_match(dev, chanId)) != NULL) {
        *data = convert_code2PhyVal(get_code2PhyVal_cfg(cfg_ptr, chan_ptr->alarm_mode)
                    , chan_ptr->low_code);
    }
    return 0;
}

static int32_t sdrv_pvt_float_data_get(pvt_dev_t* dev, pvt_float_get_enum_t type, int32_t chanId, float* data)
{
    int32_t ret = -1;

    switch (type) {
        case PVT_GET_FLOAT_CUR_TEMP:
            ret = get_float_data_cur_temp(dev, data);
            break;
        case PVT_GET_FLOAT_CUR_VOLTAGE:
            ret = get_float_data_cur_voltage(dev, data);
            break;
        case PVT_GET_FLOAT_ALARM_PHYVAL_HIGH:
            ret = get_float_data_alarm_phyval_high(dev, chanId, data);
            break;
        case PVT_GET_FLOAT_ALARM_PHYVAL_LOW:
            ret = get_float_data_alarm_phyval_low(dev, chanId, data);
            break;
        default:
            SDRV_PVT_ERR("invalid dta type for %s\n", __FUNCTION__);
            break;
    }

    return ret;
}

static int32_t set_integer_data_sample_mode(pvt_dev_t* dev, int32_t data)
{
    if (data >= PVT_SENSOR_SAMPLE_MODE_INVALID ||
        data < 0) {
        SDRV_PVT_ERR("invalid parameter for %s\n", __FUNCTION__);
        return -1;
    }

    sdrv_pvt_dev_t* sdrv_dev_ptr = dev->priv;
    // check is in alarm mode
    if (sdrv_dev_ptr->chan_bit != 0) {
        SDRV_PVT_ERR("please clean all alarm channel(0x%x) or reset device before change sample mode\n", sdrv_dev_ptr->chan_bit);
        return -1;
    }

    sdrv_dev_ptr->sensor_mode = data;
    set_sensor_sample_mode(dev, data);

    return 0;
}

static int32_t set_integer_data_sensor_pntype(pvt_dev_t* dev, int32_t data)
{
    if (data >= PVT_SENSOR_PN_TYPE_NUM ||
        data < 0) {
        SDRV_PVT_ERR("invalid parameter for %s\n", __FUNCTION__);
        return -1;
    }

    sdrv_pvt_dev_t* sdrv_dev_ptr = dev->priv;
    sdrv_dev_ptr->sensor_type = data;

    return 0;
}

static int32_t sdrv_pvt_integer_data_set(pvt_dev_t* dev, pvt_integer_set_enum_t type, int32_t data)
{
    if (dev == NULL ||
        type >= PVT_SET_INT_INVALID ) {
        SDRV_PVT_ERR("parameter error for %s\n", __FUNCTION__);
        return -1;
    }

    sdrv_pvt_dev_t* sdrv_dev_ptr = dev->priv;
    irq_state_t state;
    int32_t ret = -1;

    state = spin_lock_irqsave(&sdrv_dev_ptr->lock);

    switch (type) {
        case PVT_SET_INT_SAMPLE_MODE:
            ret = set_integer_data_sample_mode(dev, data);
            break;
        case PVT_SET_INT_SENSOR_PNTYPE:
            ret = set_integer_data_sensor_pntype(dev, data);
            break;
        default:
            break;
    }

    spin_unlock_irqrestore(&sdrv_dev_ptr->lock, state);
    return ret;
}

pvt_dev_t* sdrv_pvt_probe(sdrv_pvt_dev_t* sdrv_pvt_ptr, const sdrv_pvt_config_t* cfg_ptr)
{
    pvt_dev_t* dev;

    if (sdrv_pvt_ptr == NULL ||
        cfg_ptr == NULL) {
        SDRV_PVT_ERR("invalid parameter for pvt device probeing \n");
        return NULL;
    }

    sdrv_pvt_ptr->cfg_ptr = cfg_ptr;
    dev = &sdrv_pvt_ptr->dev;
    dev->base = cfg_ptr->base;
    dev->ops = &s_sdrv_pvt_ops;
    dev->priv = sdrv_pvt_ptr;

    pvt_init(dev);

    return dev;
}

/* end of file sdrv_pvt_drv.c */