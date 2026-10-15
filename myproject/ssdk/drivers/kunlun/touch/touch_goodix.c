/**
 * @file touch_goodix.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: touch goodix implement file
 *
 * Revision History:
 * -----------------
*/
/* =========================== include file session ======================== */
#include <compiler.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <board.h>

#include <driver/gpio.h>
#include <timer_cfg.h>

#include <driver/kunlun/touch/touch.h>
#include <driver/kunlun/touch/touch_vendor.h>

/* =========================== marco define session ======================== */
/** macro define section */
#define STRUCT_MEMBER_OFFSET(x, y)         (size_t)(&((x *)(0))->y)

#define GOODIX_EVENT_COMPLETE        0x1
#define GOODIX_EVENT_DEINIT          0x2
#define GOODIX_EVENT_DEV_RESET       0x4

#define GOODIX_I2C_TRANS_MAX_LEN     256
#define GOODIX_CONTACT_SIZE         8
#define GOODIX_MAX_CONTACTS         10
#define GOODIX_CFG_NAME             "sts_cfg.bin"
#define GOODIX_CONFIG_MAX_LENGTH    240
#define GOODIX_CONFIG_911_LENGTH    186
#define GOODIX_CONFIG_967_LENGTH    228
#define GOODIX_VENDOR_ID            0x0416
#define GTP_READ_COOR_ADDR          0x814E
#define GTP_READ_POINT_COORD_ADDR   0x814F
#define GOODIX_REG_ID               0x8140

/** log macro define */
#define TOUCH_DEBUG_LVL      7
#define EXTRA_FMT   "touch: "

#if TOUCH_DEBUG_LVL >= SSDK_ERR
#define TOUCH_ERR(format, ...) \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define TOUCH_ERR(format, ...)
#endif

#if TOUCH_DEBUG_LVL >= SSDK_WARNING
#define TOUCH_WARN(format, ...) \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define TOUCH_WARN(format, ...)
#endif

#if TOUCH_DEBUG_LVL >= SSDK_INFO
#define TOUCH_INFO(format, ...) \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define TOUCH_INFO(format, ...)
#endif

/* =========================== structure define session ==================== */
/** i2c transfer structure */
typedef struct i2c_transfer {
    uint8_t reg_h;
    uint8_t reg_l;
    uint8_t val[GOODIX_I2C_TRANS_MAX_LEN];
} __PACKED i2c_transfer_t;

/** sdrv goodix gt928 driver structure */
typedef struct sdrv_touch_drv {
    touch_dev_t dev;
    uint16_t id;
    uint16_t version;
    uint16_t vendor;
    osEventFlagsId_t event;
    osMutexId_t mutex;
    osThreadId_t thread;
    bool is_run;

    touch_cb_chan_t cb[TOUCH_NOTICER_MAX_NUM];
}sdrv_touch_drv_t;


/* =========================== function declare session ==================== */
static int32_t goodix_init(touch_dev_t* dev);
static int32_t goodix_deinit(touch_dev_t* dev);
static int32_t goodix_online_check(touch_dev_t* dev);
static int32_t goodix_register_callback(touch_dev_t* dev
                                        , touch_event_notice cb
                                        , void* usrCtx);
static int32_t goodix_touch_control(touch_dev_t* dev
                                    , touch_ctrl_type_enum_t type
                                    , void* data
                                    , int32_t* len);
static int32_t goodix_register_read(touch_dev_t* dev
                                    , uint32_t reg
                                    , uint32_t* regVal);
static int32_t goodix_register_write(touch_dev_t* dev
                                    , uint32_t reg
                                    , uint32_t regVal);

/* =========================== static value define session ================= */
static touch_ops_t s_goodix_ops = {
    .init = goodix_init,
    .deinit = goodix_deinit,
    .online_check = goodix_online_check,
    .register_callback = goodix_register_callback,
    .device_control = goodix_touch_control,
    .reg_read = goodix_register_read,
    .reg_write = goodix_register_write,
};

/* =========================== function implementation session ============= */
#ifndef TOUCH_I2C_POLLING
static int touch_i2c_read(i2c_adap_dev_t *adap, uint8_t addr, i2c_transfer_t* buf, int len)
{
    int ret = 0;
    ret = i2c_write_read(adap, addr
                         , (uint8_t *)buf, STRUCT_MEMBER_OFFSET(i2c_transfer_t, val)
                         , buf->val, len);
    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s: read error, ret=%d\n", __func__, ret);
        return ret;
    }

    return 0;
}

static int touch_i2c_write(i2c_adap_dev_t *adap, uint8_t addr, i2c_transfer_t* buf, int len)
{
    int ret = 0;
    ret = i2c_write(adap, addr, (uint8_t *)buf, len + STRUCT_MEMBER_OFFSET(i2c_transfer_t, val));
    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s: write error, ret=%d.\n", __func__, ret);
        return ret;
    }

    return ret;
}
#else
int touch_i2c_read(i2c_adap_dev_t* adap, uint8_t addr, i2c_transfer_t* buf, int len)
{
    int ret;
    struct i2c_msg msgs[2] = {
        {
            .addr = addr,
            .buf = (uint8_t *)buf,
            .len = STRUCT_MEMBER_OFFSET(i2c_transfer_t, val),
            .addr_flag = I2C_ADDR7B,
            .rw_flag = I2C_M_W,
            .polling = 1,
        },
        {
            .addr = addr,
            .buf = buf->val,
            .len = len,
            .addr_flag = I2C_ADDR7B,
            .rw_flag = I2C_M_R,
            .polling = 1,
        },
    };
    ret = i2c_transfer(adap, msgs, 2);
    return ret;
}

int touch_i2c_write(i2c_adap_dev_t *adap, uint8_t addr, i2c_transfer_t* buf, int len)
{
    int ret;
    struct i2c_msg msgs[1] = {
        {
            .addr = addr,
            .buf = buf,
            .len = len + STRUCT_MEMBER_OFFSET(i2c_transfer_t, val),
            .addr_flag = I2C_ADDR7B,
            .rw_flag = I2C_M_W,
            .polling = 1,
        },
    };
    ret = i2c_transfer(adap, msgs, 1);
    return ret;
}
#endif

static int32_t coord_point_convert(ts_coord_cfg_t* cfgPtr
                                   , uint16_t oriX, uint16_t oriY, uint16_t oriW
                                   , ts_coord_point_t* pointPtr)
{
    if (cfgPtr == NULL || pointPtr == NULL) {
        TOUCH_ERR("invalid parameter for %s\n", __func__);
        return -1;
    }

    uint16_t x = 0, y = 0;

    // check swapped
    if (cfgPtr->swapped_x_y) {
        x = oriY;
        y = oriX;
    }
    else {
        x = oriX;
        y = oriY;
    }

    // calculate offset
    x -= cfgPtr->inverted_x;
    y -= cfgPtr->inverted_y;

    // check boundary
    if (x > cfgPtr->x_max ||
        y > cfgPtr->y_max) {
        TOUCH_WARN("coord point(%d, %d) is overrange for config(%d, %d), it will be ignoreed\n"
                    , x, y, cfgPtr->x_max, cfgPtr->y_max);
        return -1;
    }

    pointPtr->x = x;
    pointPtr->y = y;
    pointPtr->w = oriW;

    return 0;
}

static int goodix_ts_read_input_report(touch_dev_t* dev, ts_coord_info_t* coordPtr)
{
    int32_t ret = -1;
    int32_t count = 0;
    int32_t touch_num = 0;

    i2c_transfer_t end_cmd = {
        .reg_h = GTP_READ_COOR_ADDR >> 8,
        .reg_l = GTP_READ_COOR_ADDR & 0xFF,
        .val = {0}
    };
    i2c_transfer_t touch_data = {
        .reg_h = GTP_READ_COOR_ADDR >> 8,
        .reg_l = GTP_READ_COOR_ADDR & 0xFF,
        .val = {0}
    };
    i2c_transfer_t point_data = {
        .reg_h = GTP_READ_POINT_COORD_ADDR >> 8,
        .reg_l = GTP_READ_POINT_COORD_ADDR & 0xFF,
        .val = {0}
    };

    memset(coordPtr, 0x00, sizeof(ts_coord_info_t));

    while (count < 10) {
        touch_i2c_read(dev->cfg_ptr->i2c_dev, dev->cfg_ptr->i2c_addr, &touch_data, 1);

        if (touch_data.val[0] & (0x1 << 7)) {
            // read touch number
            touch_num = touch_data.val[0] & 0xf;

            if (touch_num > 0) {
                osDelay(1);
                touch_i2c_read(dev->cfg_ptr->i2c_dev, dev->cfg_ptr->i2c_addr, &point_data, GOODIX_CONTACT_SIZE * touch_num + 1);

                coordPtr->touch_num = touch_num;
                coordPtr->have_key = (touch_data.val[0] >> 4) & 0x1;
                coordPtr->key_value = point_data.val[GOODIX_CONTACT_SIZE * touch_num];

                uint8_t point_num = 0;
                for (int32_t cnt = 0; cnt < touch_num; cnt++) {

                    coordPtr->point[point_num].id = point_data.val[cnt * GOODIX_CONTACT_SIZE];

                    uint16_t x = point_data.val[cnt * GOODIX_CONTACT_SIZE + 1] |
                                 (point_data.val[cnt * GOODIX_CONTACT_SIZE + 2] << 8);
                    uint16_t y = point_data.val[cnt * GOODIX_CONTACT_SIZE + 3] |
                                 (point_data.val[cnt * GOODIX_CONTACT_SIZE + 4] << 8);

                    uint16_t w = point_data.val[cnt * GOODIX_CONTACT_SIZE + 5] |
                                 (point_data.val[cnt * GOODIX_CONTACT_SIZE + 6] << 8);

                    if (coord_point_convert(&dev->cfg_ptr->coord_config, x, y, w, &coordPtr->point[point_num]) == 0) {
                        point_num++;
                    }
                    else {
                        // do nothing, get next point
                    }
                }
                coordPtr->touch_num = point_num;
                ret = 0;
            }
            else if (touch_num == 0) {
                coordPtr->touch_num = 0;
                ret = 0;
            }
            else {
                // retry
            }
            break;
        }

        count++;
        osDelay(1);
    }

    touch_i2c_write(dev->cfg_ptr->i2c_dev, dev->cfg_ptr->i2c_addr, &end_cmd, 3);
    osDelay(1);
    return ret;
}

static void goodix_irq_handler(struct gpio_dev *dev, int pin, void *arg)
{
    touch_dev_t* touch_dev = arg;
    sdrv_touch_drv_t* touch_drv = touch_dev->priv;

    gpio_int_enable(dev, pin, false);
    osEventFlagsSet(touch_drv->event, GOODIX_EVENT_COMPLETE);
}


static int32_t goodix_reset_device(touch_dev_t* dev)
{
    serdes_gpio_ctrl_t gpio_ctrl;
    uint8_t len = sizeof(gpio_ctrl);

    ssdk_printf(SSDK_EMERG, "touch: %s reset device start! \n", dev->name);
    if (dev->cfg_ptr->serdes_config.enable) {

        gpio_ctrl.io_num = dev->cfg_ptr->serdes_config.reset_pin;
        gpio_ctrl.dir = 0;
        gpio_ctrl.val = 0;
        serdes_control(*dev->cfg_ptr->serdes_config.serdes
                            , DES_CONTROL_GPIO_CTRL
                            , &gpio_ctrl
                            , &len);

        gpio_ctrl.io_num = dev->cfg_ptr->serdes_config.irq_pin;
        gpio_ctrl.dir = 0;
        gpio_ctrl.val = 0;
        serdes_control(*dev->cfg_ptr->serdes_config.serdes
                            , DES_CONTROL_GPIO_CTRL
                            , &gpio_ctrl
                            , &len);

        gpio_ctrl.io_num = dev->cfg_ptr->serdes_config.irq_pin;
        gpio_ctrl.dir = 0;
        gpio_ctrl.val = (dev->cfg_ptr->i2c_addr == 0x14);
        serdes_control(*dev->cfg_ptr->serdes_config.serdes
                            , DES_CONTROL_GPIO_CTRL
                            , &gpio_ctrl
                            , &len);
        osDelay(1);

        gpio_ctrl.io_num = dev->cfg_ptr->serdes_config.reset_pin;
        gpio_ctrl.dir = 0;
        gpio_ctrl.val = 1;
        serdes_control(*dev->cfg_ptr->serdes_config.serdes
                            , DES_CONTROL_GPIO_CTRL
                            , &gpio_ctrl
                            , &len);
        osDelay(5);

        gpio_ctrl.io_num = dev->cfg_ptr->serdes_config.irq_pin;
        gpio_ctrl.dir = 0;
        gpio_ctrl.val = 0;
        serdes_control(*dev->cfg_ptr->serdes_config.serdes
                            , DES_CONTROL_GPIO_CTRL
                            , &gpio_ctrl
                            , &len);
        osDelay(50);

        gpio_ctrl.io_num = dev->cfg_ptr->serdes_config.irq_pin;
        gpio_ctrl.dir = 1;
        serdes_control(*dev->cfg_ptr->serdes_config.serdes
                            , DES_CONTROL_GPIO_CTRL
                            , &gpio_ctrl
                            , &len);

        gpio_ctrl.io_num = dev->cfg_ptr->serdes_config.irq_pin;
        gpio_ctrl.dir = 1;
        serdes_control(*dev->cfg_ptr->serdes_config.serdes
                            , SER_CONTROL_GPIO_CTRL
                            , &gpio_ctrl
                            , &len);

        serdes_port_passthrough_ctl_t port_ctl;
        port_ctl.dir = 1;   /**< des -> ser */
        port_ctl.d_port = dev->cfg_ptr->serdes_config.irq_pin;
        port_ctl.s_port = dev->cfg_ptr->serdes_config.sirq_pin;
        uint8_t ctl_len = sizeof(port_ctl);
        serdes_control(*dev->cfg_ptr->serdes_config.serdes
                        , SERDES_PORT_PASSTHROUGH_CTL
                        , &port_ctl
                        , &ctl_len);
    }

    ssdk_printf(SSDK_EMERG, "touch: %s reset device end! \n", dev->name);
    return 0;
}

void touch_test_send_flg(touch_dev_t* dev)
{
    sdrv_touch_drv_t* sdrv_touch = (sdrv_touch_drv_t*)dev->priv;
    osEventFlagsSet(sdrv_touch->event, GOODIX_EVENT_COMPLETE);
}

static int goodix_read_version(touch_dev_t* dev)
{
    char id_str[5];
    int id, version;
    sdrv_touch_drv_t* sdrv_touch = (sdrv_touch_drv_t*)dev->priv;
    i2c_transfer_t ver_data = {
        .reg_h = GOODIX_REG_ID >> 8,
        .reg_l = GOODIX_REG_ID & 0xFF,
    };
    memset(ver_data.val, 0, 6);

    touch_i2c_read(dev->cfg_ptr->i2c_dev, dev->cfg_ptr->i2c_addr, &ver_data, 8);
    TOUCH_INFO("%s(): instance(%#x), %#x, %#x, %#x, %#x, %#x, %#x.\n",
            __func__, dev->cfg_ptr->screen_id
            , ver_data.val[0], ver_data.val[1], ver_data.val[2]
            , ver_data.val[3], ver_data.val[4], ver_data.val[5]);

    memcpy(id_str, ver_data.val, 4);
    id = atoi(id_str);
    version = ver_data.val[4] + (ver_data.val[5] << 8);

    if (id == 0)
        return -1;
    else {
        sdrv_touch->id = id;
        sdrv_touch->version = version;
        sdrv_touch->vendor = GOODIX_VENDOR_ID;
        // TOUCH_INFO("[debug]read goodix version info: %d id: %d\n", version, id);
    }

    return 0;
}

static void goodix_ts_work_func(void *arg)
{
    ts_coord_info_t report_data;
    touch_dev_t* dev = arg;
    sdrv_touch_drv_t* sdrv_touch = dev->priv;
    touch_notice_t notice_data;
    uint32_t event_flg = 0;

    TOUCH_ERR("goodix work function entry\n");
    // touch reset
    int32_t count = 0;
     do {
        goodix_reset_device(dev);
        if (goodix_read_version(dev) == 0)
            break;
    } while (++count < 3);
    if (count >= 3) {
        TOUCH_ERR("read touch version error after device reset. \n");
    }

    sdrv_touch->is_run = true;
    while (sdrv_touch->is_run) {
        event_flg = osEventFlagsWait(sdrv_touch->event
                                     , GOODIX_EVENT_COMPLETE | GOODIX_EVENT_DEINIT | GOODIX_EVENT_DEV_RESET
                                     , osFlagsWaitAny
                                     , osWaitForever);

        osMutexAcquire(sdrv_touch->mutex, osWaitForever);

        if (event_flg & GOODIX_EVENT_COMPLETE) {
            if (goodix_ts_read_input_report(dev, &report_data) == 0) {
                TOUCH_INFO("%s: [%d] 0x%x, (%d, %d, %d, %d)\n", __func__, dev->cfg_ptr->screen_id
                            , report_data.touch_num
                            , report_data.point[0].id
                            , report_data.point[0].x
                            , report_data.point[0].y
                            , report_data.point[0].w);
                notice_data.evt_type = TS_NOTICE_EVT_COORD;
                memcpy(&notice_data.coord, &report_data, sizeof(report_data));
                for (int32_t i = 0; i < TOUCH_NOTICER_MAX_NUM; i++) {
                    if (!sdrv_touch->cb[i].is_valid &&
                        sdrv_touch->cb[i].cb != NULL) {
                        sdrv_touch->cb[i].cb(dev->cfg_ptr->screen_id, &notice_data, sdrv_touch->cb[i].usrCtx);
                    }
                }
            }
            else {
                // do nothing
            }
        }

       if (event_flg & GOODIX_EVENT_DEV_RESET) {
           goodix_reset_device(dev);
       }

       if (event_flg & GOODIX_EVENT_DEINIT) {
           sdrv_touch->is_run = false;
       }

        gpio_int_enable(dev->cfg_ptr->irq_pin.pin_dev, dev->cfg_ptr->irq_pin.pin_num, true);
        osMutexRelease(sdrv_touch->mutex);
    }

    TOUCH_ERR("touch task exit! \n");

    osEventFlagsDelete(sdrv_touch->event);
    osMutexDelete(sdrv_touch->mutex);
    osThreadExit();

    return ;
}

static int goodix_config_device(touch_dev_t* dev)
{
    sdrv_touch_drv_t* sdrv_touch = dev->priv;

    sdrv_touch->event = osEventFlagsNew(NULL);
    sdrv_touch->mutex = osMutexNew(NULL);

    for (int32_t i = 0; i < TOUCH_NOTICER_MAX_NUM; i++) {
        sdrv_touch->cb[i].is_valid = true;
    }
    sdrv_touch->is_run = false;

    osThreadAttr_t attr = {
        .name = dev->cfg_ptr->name,
        .priority = osPriorityRealtime,
        .stack_size = 2048,
    };
    sdrv_touch->thread = osThreadNew(goodix_ts_work_func, dev, &attr);

    gpio_config(dev->cfg_ptr->irq_pin.pin_dev
                , dev->cfg_ptr->irq_pin.pin_num
                , GPIO_INTR_FALLING_EDGE);

    gpio_attach_irq(dev->cfg_ptr->irq_pin.pin_dev
                , dev->cfg_ptr->irq_pin.pin_num
                , goodix_irq_handler
                , dev);
    return 0;
}

static int32_t goodix_init(touch_dev_t* dev)
{
    if (dev == NULL || dev->priv == NULL) {
        TOUCH_ERR("invalid parameter for %s \n", __func__);
        return -1;
    }

    // TOUCH_INFO("[debug]goodix touch device init \n");
    // serdes config
    if (dev->cfg_ptr->serdes_config.enable) {
        // check link
        if (serdes_check_serdes_link(*dev->cfg_ptr->serdes_config.serdes) != 0) {
            return -1;
        }

        // enable port
        if (serdes_control(*dev->cfg_ptr->serdes_config.serdes
                                , SER_CONTROL_PORT_ENABLE, NULL, 0) != 0) {
            return -1;
        }

        // enable passthrough
        if (serdes_control(*dev->cfg_ptr->serdes_config.serdes
                                , SER_CONTROL_ENABLE_I2C_PASSTROUGH, NULL, 0) != 0) {
            return -1;
        }
    }

    // os config
    return goodix_config_device(dev);

}

static int32_t goodix_deinit(touch_dev_t* dev)
{
    // set event for destroy thread
    if (dev == NULL) {
        TOUCH_ERR("invalid parameter for %s \n", __func__);
        return -1;
    }

    sdrv_touch_drv_t * sdrv_touch = (sdrv_touch_drv_t *)dev->priv;
    osEventFlagsSet(sdrv_touch->event, GOODIX_EVENT_DEINIT);
    return 0;
}

static int32_t goodix_online_check(touch_dev_t* dev)
{
    int32_t ret = -1;
    if (dev == NULL) {
        TOUCH_ERR("invalid parameter for %s.\n", __func__);
        return -1;
    }

    // sdrv_touch_drv_t* sdrv_touch = (sdrv_touch_drv_t *)dev->priv;

    if (dev->cfg_ptr->serdes_config.enable) {
        if (serdes_check_serdes_link(*dev->cfg_ptr->serdes_config.serdes) == 0 &&
            goodix_read_version(dev) == 0) {
            ret = 0;
        }
        else {
            TOUCH_INFO("%s touch device offline, please check serdes link status or touch link status\n", dev->name);
            return -1;
        }
    }
    else {
        if (goodix_read_version(dev) == 0) {
            ret = 0;
        }
        else {
            TOUCH_INFO("%s touch device offline, please check touch link status\n", dev->name);
            return -1;
        }
    }

    sdrv_touch_drv_t * sdrv_touch = (sdrv_touch_drv_t *)dev->priv;
    if (sdrv_touch->is_run) {
        ret = 0;
    }
    else {
        TOUCH_ERR("%s touch device offline, please check touch work task status.\n", dev->name);
        return -1;
    }

    return ret;
}

static int32_t goodix_register_callback(touch_dev_t* dev, touch_event_notice cb, void* usrCtx)
{
    if (dev == NULL || cb == NULL) {
        TOUCH_ERR("invalid parameter for %s.\n", __func__);
        return -1;
    }

    sdrv_touch_drv_t* sdrv_touch = (sdrv_touch_drv_t *)dev->priv;

    int32_t ret = -1;

    osMutexAcquire(sdrv_touch->mutex, osWaitForever);
    for (int32_t i = 0; i < TOUCH_NOTICER_MAX_NUM; i++) {
        if (sdrv_touch->cb[i].is_valid) {
            sdrv_touch->cb[i].cb = cb;
            sdrv_touch->cb[i].usrCtx = usrCtx;
            sdrv_touch->cb[i].is_valid = false;
            ret = 0;
            TOUCH_INFO("[debug]register touch data callback with chan: %d\n", i);
            break;
        }
    }

    if (ret != 0) {
        TOUCH_WARN("there is no invalid callback channel for %s \n", __func__);
    }
    osMutexRelease(sdrv_touch->mutex);

    return ret;
}

static int32_t goodix_touch_control(touch_dev_t* dev, touch_ctrl_type_enum_t type, void* data, int32_t* len)
{
    if (dev == NULL) {
        TOUCH_ERR("parameter error for %s \n", __func__);
        return -1;
    }
    serdesCtl_i2cAddr_t i2c_addr;

    sdrv_touch_drv_t* sdrv_touch = (sdrv_touch_drv_t *)dev->priv;
    // control function not implement
    switch (type) {
        case TS_CONTROL_CHANGE_MODE:
            //@TODO change sensor mode
            break;
        case TS_CONTROL_UPDATE_FIRMWARE:
            //@TODO update firmware
            break;
        case TS_CONTROL_RESET_DEV:
            ssdk_printf(SSDK_EMERG, "touch: %s reset device %s \n", __func__, dev->name);
            osEventFlagsSet(sdrv_touch->event, GOODIX_EVENT_DEV_RESET);
            break;
        case TS_CONTROL_GET_COORD_CFG:
            if (data != NULL) {
                ((tsCtrl_configInfo_t *)(data))->inverted_x = dev->cfg_ptr->coord_config.inverted_x;
                ((tsCtrl_configInfo_t *)(data))->inverted_y = dev->cfg_ptr->coord_config.inverted_y;
                ((tsCtrl_configInfo_t *)(data))->max_touch_num = dev->cfg_ptr->coord_config.max_touch_num;
                ((tsCtrl_configInfo_t *)(data))->swapped_x_y = dev->cfg_ptr->coord_config.swapped_x_y;
                if (len != NULL)
                    *len = sizeof(tsCtrl_configInfo_t);
            }
            else {
                return -1;
            }
            break;
        case TS_CONTROL_GET_SCREEN_ID:
            *((int32_t *)data) = dev->cfg_ptr->screen_id;
            *len = sizeof(int32_t);
            break;
        case TS_CONTROL_GET_VENDOR_INFO:
            if (data != NULL) {
                ((tsCtrl_vendorInfo_t *)(data))->name = dev->cfg_ptr->name;
                ((tsCtrl_vendorInfo_t *)(data))->id = sdrv_touch->id;
                ((tsCtrl_vendorInfo_t *)(data))->vendor = sdrv_touch->vendor;
                ((tsCtrl_vendorInfo_t *)(data))->version = sdrv_touch->version;
                ((tsCtrl_vendorInfo_t *)(data))->ts_addr = dev->cfg_ptr->i2c_addr;
                if (dev->cfg_ptr->serdes_config.serdes != NULL) {
                    memset(&i2c_addr, 0x00, sizeof(i2c_addr));
                    serdes_control(*dev->cfg_ptr->serdes_config.serdes, SERDES_CONTROL_GET_I2C_ADDR, &i2c_addr, NULL);
                    ((tsCtrl_vendorInfo_t *)(data))->ser_addr = i2c_addr.ser_addr;
                    ((tsCtrl_vendorInfo_t *)(data))->des_addr = i2c_addr.des_addr;
                }

                if (len != NULL)
                    *len = sizeof(tsCtrl_vendorInfo_t);
            }
            break;
        default:
            break;
    }

    return 0;
}

static int32_t goodix_register_read(touch_dev_t* dev, uint32_t reg, uint32_t* regVal)
{
    if (dev == NULL || regVal == NULL) {
        TOUCH_ERR("invalid parameter for %s \n", __func__);
        return -1;
    }

    sdrv_touch_drv_t* sdrv_touch = (sdrv_touch_drv_t *)dev->priv;
    i2c_transfer_t read_data = {
        .reg_h = (reg >> 8) & 0xFF,
        .reg_l = reg & 0xFF,
        .val = {0}
    };

    osMutexAcquire(sdrv_touch->mutex, osWaitForever);
    touch_i2c_read(dev->cfg_ptr->i2c_dev, dev->cfg_ptr->i2c_addr, &read_data, 1);
    *regVal = read_data.val[0];
    osMutexRelease(sdrv_touch->mutex);

    return 0;
}

static int32_t goodix_register_write(touch_dev_t* dev, uint32_t reg, uint32_t regVal)
{
    if (dev == NULL) {
        TOUCH_ERR("invalid parameter for %s \n", __func__);
        return -1;
    }

    sdrv_touch_drv_t* sdrv_touch = (sdrv_touch_drv_t *)dev->priv;
    i2c_transfer_t write_data = {
        .reg_h = (reg >> 8) & 0xFF,
        .reg_l = reg & 0xFF,
        .val = {regVal}
    };

    osMutexAcquire(sdrv_touch->mutex, osWaitForever);
    touch_i2c_write(dev->cfg_ptr->i2c_dev, dev->cfg_ptr->i2c_addr, &write_data, 1);
    osMutexRelease(sdrv_touch->mutex);

    return 0;
}

touch_dev_t* goodix_touch_probe(sdrv_touch_handle* touchDrv, const touch_config_t* cfgPtr)
{
    touch_dev_t* dev = NULL;
    if (cfgPtr == NULL) {
        TOUCH_ERR("invalid parameter for %s \n", __func__);
        return dev;
    }

    if (*touchDrv != NULL) {
        TOUCH_INFO("already probed touch device for %s.\n", cfgPtr->name);
        return &((sdrv_touch_drv_t *)(*touchDrv))->dev;
    }

    *touchDrv = malloc(sizeof(sdrv_touch_drv_t));
    if (*touchDrv == NULL) {
        TOUCH_ERR("malloc memory for %s device error\n", cfgPtr->name);
        return NULL;
    }
    memset(*touchDrv, 0x00, sizeof(sdrv_touch_drv_t));

    sdrv_touch_drv_t* touch_drv = (sdrv_touch_drv_t *)(*touchDrv);
    touch_drv->dev.cfg_ptr = cfgPtr;
    touch_drv->dev.name = cfgPtr->name;
    touch_drv->dev.ops = &s_goodix_ops;
    touch_drv->dev.priv = touch_drv;
    dev = &touch_drv->dev;

    if (touch_init(dev) != 0) {
        TOUCH_ERR("touch device init error\n");
    }

    return dev;
}

/* end of file touch_goodix.c */