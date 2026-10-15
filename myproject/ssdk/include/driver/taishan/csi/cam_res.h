/**
 * @file driver/taishan/csi/cam_res.h
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

enum camera_id { CAMERA_0 = 0, CAMERA_1, CAMERA_2, CAMERA_NUM };

struct camera_res_cfg {
    unsigned int gpio[4];
    unsigned int i2c_res;
    unsigned int i2c_addr;
    char *name;
    void *i2c_handle;
};

extern struct camera_res_cfg g_cam_cfg[CAMERA_NUM];

#ifdef CONFIG_CSI_IOMUX
extern void csi_iomux_cfg(int sel);
#endif

#endif /* __CAM_RES_H__ */
