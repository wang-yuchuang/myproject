/*
 * Copyright (C) 2020-2030 Semidrive, Inc. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <sys/types.h>

#include "exvdev.h"
#include "driver/kunlun/csi/cam_res.h"
#include "sdrv_serdes_core.h"

static struct serdes_dev_t g_serdes[SERDES_NUM];
spin_lock_t ginit_lock = SPIN_LOCK_INITIAL_VALUE;

#ifdef CONFIG_mipi_max9286_fengjing_MC163_720P_MAX96705
extern serdes_para_t max9286_fengjing_MC163_MAX96705_720p;
#endif

#ifdef CONFIG_mipi_max96722_sengyun_IMX390C_5200_MAX9295
extern serdes_para_t max96722_sengyun_IMX390c_5200_GMSL2_1080p;
#endif

#ifdef CONFIG_dvp_max96706_fengjing_MC163_MAX96705
extern serdes_para_t max96706_fengjing_MC163_MAX96705_720p;
#endif

#ifdef CONFIG_mipi_max96722_fengjing_MC163_MAX96705
extern serdes_para_t max96722_fengjing_MC163_MAX96705_720p;
#endif

serdes_para_t *g_serdes_config[] = {
#ifdef CONFIG_mipi_max9286_fengjing_MC163_720P_MAX96705
    &max9286_fengjing_MC163_MAX96705_720p,
#endif

#ifdef CONFIG_mipi_max96722_sengyun_IMX390C_5200_MAX9295
    &max96722_sengyun_IMX390c_5200_GMSL2_1080p,
#endif

#ifdef CONFIG_dvp_max96706_fengjing_MC163_MAX96705
    &max96706_fengjing_MC163_MAX96705_720p,
#endif

#ifdef CONFIG_mipi_max96722_fengjing_MC163_MAX96705
    &max96722_fengjing_MC163_MAX96705_720p,
#endif
};

#define mdelay(ms) osDelay(ms)

int serdes_read_reg_16(i2c_adap_dev_t *client, u8 address, u16 reg, u8 *val)
{
    int ret;


    ret = i2c_read_reg16(client, address, reg, val, 1);

    if (ret < 0) {
        serdes_err("%s: error: read reg=0x%x\n", __func__, reg);
        return ret;
    }
    return 0;
}

int serdes_write_reg_16(i2c_adap_dev_t *client, u8 address, u16 reg, u8 val)
{
    int ret;


    ret = i2c_write_reg16(client, address,reg, &val, 1);

    if (ret < 0) {
        serdes_err("%s: error: reg=0x%x, val=%x\n", __func__, reg, val);
        return ret;
    }
    return 0;
}

int serdes_read_reg_8(i2c_adap_dev_t *client, u8 address, u8 reg, u8 *val)
{
    u8 buf[1];
    int ret = 0;

    memset(buf, 0, sizeof(buf));

    ret = i2c_read_reg8(client, address, reg, buf, 1);

    if (ret < 0) {
        serdes_err("%s: error: read reg=0x%x\n", __func__, reg);
        return ret;
    }

    *val = buf[0];
    return 0;
}

int serdes_write_reg_8(i2c_adap_dev_t *client, u8 address, u8 reg, u8 val)
{
    int ret;

    ret = i2c_write_reg8(client, address, reg, &val, 1);

    if (ret < 0) {
        serdes_err("%s: error: reg=0x%x, val=%x\n", __func__, reg, val);
        return ret;
    }

    return 0;
}

static void des_hotplug_func(void *arg)
{
    struct serdes_dev_t *serdes = (struct serdes_dev_t *)arg;
    serdes_para_t *pPara;
    bool fg_reinit = false;

    if (serdes == NULL) {
        serdes_err("%s %dNo serdes param\n", __func__, __LINE__);
        return ;
    }

    pPara = serdes->priv;

    if (pPara == NULL) {
        return ;
    }

    while (1) {
        if (serdes->hotplug_should_exit) {
            break;
        }

        if (pPara->power_on_flag == 0) {
            mdelay(500);
            continue;
        }

        mdelay(2000);

        osMutexAcquire(serdes->lock, osWaitForever);

        if (serdes->serdes_streamon_bitmap > 0) {

            if (pPara->serdes_check_link_status != NULL) {
                serdes->current_lock_status =  pPara->serdes_check_link_status(serdes);
            }

            if (pPara->serdes_check_video_lock_status != NULL) {
                fg_reinit = pPara->serdes_check_video_lock_status(serdes);
            }

            if (serdes->current_lock_status > serdes->last_lock_status || fg_reinit) {
                serdes_info("hotplug detected! last 0x%x current 0x%x\n",
                           serdes->last_lock_status,
                           serdes->current_lock_status);
                mdelay(200);//wait for link stable

                if (pPara->serdes_power != NULL){
                    //here from serdesializer power off to power on,we need some delay time to make
                    //the action stable.need to be implemented in the serdesializer driver
                    pPara->serdes_power(serdes, 0);
                    pPara->serdes_power(serdes, 1);
                }

                if (pPara->serdes_init != NULL) {
                    pPara->serdes_init(serdes);
                }

                if (pPara->serdes_start != NULL) {
                    pPara->serdes_start(serdes, 1);
                }
            }

            if (serdes->current_lock_status != serdes->last_lock_status) {
                serdes->last_lock_status = serdes->current_lock_status;
            }
        }

        osMutexRelease(serdes->lock);
    }

    serdes->hotplug_exited = 1;
    serdes_info("thread %d des_hotplug_func exit\n", serdes->serdes_id);

    return ;
}

static int serdes_s_power(struct vdev_device *vdev, int enable)
{
    struct serdes_dev_t *serdes;
    serdes_para_t *pPara;
    int ret = 0;
#ifdef PROBE_INIT
    int link = 0;
#endif

    if (vdev == NULL) {
        serdes_err("vdev null\n");
        return -1;
    }

    serdes = (struct serdes_dev_t *)vdev->priv_data;

    if (serdes == NULL) {
        serdes_err("serdes null\n");
        return -1;
    }

    pPara = (serdes_para_t *)serdes->priv;

    if (pPara == NULL) {
        serdes_err("serdes param null\n");
        return -1;
    }

    if (enable == 1) {

        osMutexAcquire(serdes->lock, osWaitForever);
#ifdef PROBE_INIT

        if (serdes->serdes_status.init_status != SERDES_DONE) {
            if(pPara->serdes_power)
                pPara->serdes_power(serdes, 1);
            pPara->power_on_flag = 1;

            if (pPara->serdes_check_link_status)
                serdes->last_lock_status = pPara->serdes_check_link_status(serdes);
            if (pPara->serdes_init != NULL) {
                pPara->serdes_init(serdes);
            }
        }

        if (pPara->serdes_check_link_status_ex != NULL) {
            link = pPara->serdes_check_link_status_ex(serdes);//check if lock is on

            if (link == 0)
                ret = -ENODEV;//current channel not link locked.
        }

#else

        if (serdes->serdes_status.init_status == SERDES_DONE) { //already inited,return
            serdes_err("already inited return\n");
            osMutexRelease(serdes->lock);
            return 0;
        }

        if (pPara->serdes_init != NULL) {
            ret = pPara->serdes_init(serdes);
        }


#endif
        serdes->serdes_status.init_status = SERDES_DONE;
        osMutexRelease(serdes->lock);
    }
    else {//do nothing,will revise later

    }

    return ret;
}

static int serdes_enum_mbus_fmt(struct vdev_device *vdev,
                               struct vdev_mbus_framefmt *fme)
{
    if ((vdev == NULL) || (fme == NULL)) {
        return -1;
    }

    if (fme->index > 0) {
        fme->index = 0;
        return -1;
    }

    fme->code = vdev->fmt.code;
    fme->field = vdev->fmt.field;
    fme->colorspace = vdev->fmt.colorspace;
    return 0;
}

static int serdes_enum_frame_size(struct vdev_device *vdev,
                                 struct vdev_frame_size_enum *fse)
{
    if (!vdev || !fse) {
        return -1;
    }

    if (fse->index > 0) {
        fse->index = 0;
        return -1;
    }

    fse->min_width = vdev->fmt.width;
    fse->max_width = vdev->fmt.width;
    fse->min_height = vdev->fmt.height;
    fse->max_height = vdev->fmt.height;

    return 0;
}

static int serdes_enum_frame_interval(struct vdev_device *vdev,
                                     struct vdev_frame_interval_enum *fie)
{
    return 0;
}

static int serdes_get_frame_interval(struct vdev_device *vdev,
                                    struct vdev_fract *fi)
{
    if (!vdev || !fi) {
        return -1;
    }

    *fi = vdev->frame_interval;
    return 0;
}

static int serdes_set_frame_interval(struct vdev_device *vdev,
                                    struct vdev_fract fi)
{
    return 0;
}

static int serdes_get_fmt(struct vdev_device *vdev,
                         struct vdev_mbus_framefmt *fmt)
{
    if (!vdev) {
        return -1;
    }

    *fmt = vdev->fmt;
    return 0;
}

static int serdes_set_fmt(struct vdev_device *vdev,
                         struct vdev_mbus_framefmt fmt)
{
    return 0;
}

static int serdes_s_stream(struct vdev_device *vdev, int enable)
{
    struct serdes_dev_t *serdes;
    serdes_para_t *pPara;

    if (!vdev) {
        return -1;
    }

    serdes = (struct serdes_dev_t *)vdev->priv_data;

    if (serdes == NULL) {
        serdes_err("serdes null\n");
        return -1;
    }

    pPara = (serdes_para_t *)serdes->priv;

    if (pPara == NULL) {
        serdes_err("serdes param null\n");
        return -1;
    }

    osMutexAcquire(serdes->lock, osWaitForever);

    if (enable == 1) {

        if (serdes->serdes_status.stream_cnt == 0) {
            if (pPara->serdes_start != NULL) {
                pPara->serdes_start(serdes, true);
            }
        }

        serdes->serdes_status.stream_cnt++;

        if (serdes->sync == 1) {
            serdes->serdes_streamon_bitmap = 0xf;
        }
        else {
            serdes->serdes_streamon_bitmap |= (1 << vdev->ch_id);
        }

        serdes->last_lock_status |= serdes->serdes_streamon_bitmap;
        serdes_info("stream on map 0x%x ch id %d\n", serdes->serdes_streamon_bitmap,
                   vdev->ch_id);
    }
    else {
        serdes->serdes_status.stream_cnt--;

        if ((serdes->sync == 1)
                && (serdes->serdes_status.stream_cnt ==
                    0)) { //set stream on bitmap to zero only when no stream is on
            serdes->serdes_streamon_bitmap = 0;
        }
        else {
            serdes->serdes_streamon_bitmap &= ~(1 << vdev->ch_id);
        }

        serdes_info("stream off map 0x%x\n", serdes->serdes_streamon_bitmap);

        if (serdes->serdes_status.stream_cnt == 0) {
            if (pPara->serdes_start != NULL) {
                pPara->serdes_start(serdes, false);
            }
        }
    }

    osMutexRelease(serdes->lock);
    return 0;
}

static int serdes_deinit(struct vdev_device *vdev)
{
    struct serdes_dev_t *serdes;

    if (!vdev) {
        return -1;
    }

    serdes = (struct serdes_dev_t *)vdev->priv_data;

    osMutexAcquire(serdes->lock, osWaitForever);
    serdes->serdes_status.ref_count--;

#ifndef PROBE_INIT

    if (serdes->serdes_status.ref_count == 0) {
        serdes->serdes_status.init_status = SERDES_SENSOR_LOCK_READY;

        if (serdes->hotplug_thread != NULL) {
            serdes->hotplug_should_exit = 1;
            osMutexRelease(serdes->lock);

            while (serdes->hotplug_exited != 1) { //wait hotplug thread exit
                thread_sleep(200);
            }

            osMutexAcquire(serdes->lock, osWaitForever);
            serdes->hotplug_thread = NULL;
        }

        hal_i2c_release_handle(serdes->i2c_client);
        serdes->i2c_client = NULL;
    }

#endif
    osMutexRelease(serdes->lock);


    return 0;
}

static const struct vdev_dev_ops des_vdev_ops = {
    .s_power = serdes_s_power,
    .enum_mbus_fmt = serdes_enum_mbus_fmt,
    .enum_frame_size = serdes_enum_frame_size,
    .enum_frame_interval = serdes_enum_frame_interval,
    .g_frame_interval = serdes_get_frame_interval,
    .s_frame_interval = serdes_set_frame_interval,
    .get_fmt = serdes_get_fmt,
    .set_fmt = serdes_set_fmt,
    .s_stream = serdes_s_stream,
    .close = serdes_deinit,
};

#define MAX_THRED_NAME (20)
struct vdev_device *serdes_init_interface(void *data)
{
    unsigned int i = 0;
    u8 channel = 0;
    struct serdes_dev_t *serdes;
    struct camera_res_cfg *cfg = (struct camera_res_cfg *)data;
    struct mipi_config *mipi_cfg = cfg->mipi_cfg;
    serdes_para_t *pPara = NULL;
    char thread_name[MAX_THRED_NAME];
     osThreadAttr_t attr = {
        .priority = osPriorityNormal,
    };

    if (!data)
        return NULL;

    if (cfg->serdes_id >= SERDES_NUM) {
        serdes_err("%s: serdes_id error: %d exceeds max id %d\n", __func__, cfg->serdes_id,
                  SERDES_NUM);
        return NULL;
    }

    if (cfg->channel >= MAX_SERAILIZER_NUM) {
        serdes_err("current channel %d exceed the max value %d\n", cfg->channel,
                  MAX_SERAILIZER_NUM);
        return NULL;
    }

    spin_lock(&ginit_lock);
    serdes = &g_serdes[cfg->serdes_id];
    pPara = serdes->priv;

    if (serdes->serdes_status.init_status == SERDES_IDLE) {
        serdes->dev_init_lock = osMutexNew(NULL);
        serdes->serdes_status.init_status = SERDES_INIT_LOCK_READY;
    }

    spin_unlock(&ginit_lock);

    serdes_info("init dev lock for %s\n", cfg->module_name);

    osMutexAcquire(serdes->dev_init_lock, osWaitForever);

    //sensor lock is created only once. deinit won't delete
    if (serdes->serdes_status.init_status == SERDES_INIT_LOCK_READY) {
        serdes->lock = osMutexNew(NULL);
        serdes->serdes_status.init_status = SERDES_SENSOR_LOCK_READY;
    }

    osMutexRelease(serdes->dev_init_lock);

    osMutexAcquire(serdes->lock, osWaitForever);

    if (serdes->serdes_status.init_status >=
            SERDES_SENSOR_READY) { //serdesializer already inited,only need to config vdev value
        if ((mipi_cfg && (mipi_cfg->sync == 1)) || (mipi_cfg == NULL)) {
            serdes->serdes_status.ref_count++;
            osMutexRelease(serdes->lock);
            return &serdes->vdev[0];
        }
        else {
            goto vdev_init;
        }
    }

    //parameters from cfg data
    serdes->i2c_client = cfg->i2c_handle;
    serdes->addr_des = cfg->i2c_addr;
    serdes->addr_poc = cfg->poc_i2c_addr;
    serdes->addr_extgpio = cfg->gpioext_addr;
    serdes->power_gpio = cfg->power_gpio;
    serdes->poc_gpio = cfg->poc_gpio;
    serdes->addr_ser = cfg->ser_i2c_addr;
    serdes->gpioext_name = cfg->gpioext_name;
    serdes->module_name = cfg->module_name;
    serdes->serdes_id = cfg->serdes_id;

    if (mipi_cfg) {
        serdes->sync = mipi_cfg->sync;
    }
    else {
        serdes->sync = 0;//parallel sync=0
    }

    for (i = 0; i < ARRAY_SIZE(g_serdes_config); i++) {
        if (strcmp(serdes->module_name, g_serdes_config[i]->module_name) == 0) {
            serdes->priv = g_serdes_config[i];
            break;
        }
    }

    if (serdes->priv == NULL) {
        serdes_err("No dev config detected\n");
        goto err;
    }

    pPara = serdes->priv;
    serdes->hotplug_should_exit = 0;
    serdes->hotplug_exited = 0;
    sprintf(thread_name, "des_thread%d", cfg->serdes_id);
    serdes->hotplug_thread = (void *)osThreadNew(des_hotplug_func,
                            (void*)serdes, &attr);

    if (serdes->hotplug_thread == NULL) {
        serdes_err("hotplug thread failed\n");
    }

    serdes->serdes_status.init_status = SERDES_SENSOR_READY;
    //vdev init
vdev_init:
    channel = cfg->channel;
    serdes->channel = channel;
    serdes->vdev[channel].priv_data = (void *)serdes;
    serdes->vdev[channel].ep.bus_type = pPara->bus_type;
    serdes->vdev[channel].frame_interval.numerator = pPara->numerator;
    serdes->vdev[channel].fmt.field = pPara->field;
    serdes->vdev[channel].fmt.colorspace = pPara->colorspace;

    serdes->vdev[channel].fmt.code = pPara->mbus_code[channel];
    serdes->vdev[channel].frame_interval.denominator = pPara->fps[channel];
    serdes->vdev[channel].fmt.width = cfg->width;
    serdes->vdev[channel].fmt.height = cfg->height;
    serdes->vdev[channel].ops = des_vdev_ops;
    serdes->vdev[channel].ch_id = channel;
    serdes->serdes_status.ref_count++;
    osMutexRelease(serdes->lock);
    return &serdes->vdev[channel];

err:
    osMutexRelease(serdes->lock);
    return NULL;
}

struct external_camera_api serdes_interface = {
    .name = "serdes_dev",
    .init = serdes_init_interface,
};
