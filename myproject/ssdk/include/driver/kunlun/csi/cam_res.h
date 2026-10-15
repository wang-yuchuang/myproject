/**
 * @file cam_res.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef __CAM_RES_H__
#define __CAM_RES_H__

#include <stdio.h>
#ifdef CONFIG_KUNLUN_CAMERA_SERVICE
#include "cam_conf.h"
#endif

enum camera_id {
    CAMERA_0 = 0,
    CAMERA_1,
    CAMERA_2,
    CAMERA_3,
    CAMERA_4,
    CAMERA_5,
    CAMERA_6,
    CAMERA_7,
    CAMERA_8,
    CAMERA_NUM
};

enum mipicsi_id {
    MIPI_CSI0 = 0,
    MIPI_CSI1,
    MIPI_CSI_NUM,
};

enum serdes_id {
    SERDES_0 = 0,
    SERDES_1,
    SERDES_2,
    SERDES_NUM,
};

struct mipi_config {
    unsigned int sync;
    unsigned int lane_num;
    unsigned int lanerate;
    unsigned int hsa;
    unsigned int hbp;
    unsigned int hsd;
    unsigned int output_type;
};

struct camera_res_cfg {
    int power_gpio;
    int poc_gpio;
    int gpi_gpio;
    unsigned int i2c_res;
    unsigned int width;
    unsigned int height;
    uint8_t i2c_addr;
    uint8_t poc_i2c_addr;
    uint8_t gpioext_addr;
    uint8_t ser_i2c_addr;
    uint8_t channel;            //deserializer channel id,denoting which channel the camera is using
    uint8_t serdes_id;          //denoting which deserializer the camera is using
    char *name;            //camera driver name
    char *module_name;     //camera module name
    char *gpioext_name;
    void *i2c_handle;
    struct mipi_config *mipi_cfg;
};

extern struct camera_res_cfg g_cam_cfg[CAMERA_NUM];
extern struct mipi_config g_mipi_cfg[MIPI_CSI_NUM];
#ifdef CONFIG_KUNLUN_CAMERA_SERVICE
extern struct cam_config cam_confs[CAMERA_NUM];
#endif

#endif /* __CAM_RES_H__ */
