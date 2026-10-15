/**
 * @file csi_drv.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: csi driver interface
 *
 * Revision History:
 * -----------------
 */
#ifndef __CSI_DRV_H__
#define __CSI_DRV_H__

struct csi_ch_params {
    uint32_t bus_type;
    uint32_t bus_fmt;
    uint32_t field_type;
    uint32_t width;
    uint32_t height;
    uint32_t width_even;
    uint32_t height_even;
    uint32_t crop_en;
    uint32_t crop_x;
    uint32_t crop_y;
    uint32_t crop_w;
    uint32_t crop_h;
    uint32_t set_pix_fmt;
    uint32_t pix_fmt_num;
    uint32_t pix_fmt[8];
};

struct csi_channel_ops {
    int (*channel_open)(uint32_t id, uint32_t ch);
    int (*channel_close)(uint32_t id, uint32_t ch);
    int (*channel_stream)(uint32_t id, uint32_t ch, int enable);
    int (*channel_qbuf)(uint32_t id, uint32_t ch, void *buf);
    int (*channel_dqbuf)(uint32_t id, uint32_t ch, void *buf);
    int (*channel_cfg_bus_type)(uint32_t id, uint32_t ch,
                                struct csi_ch_params *param);
    int (*channel_cfg_bus_fmt)(uint32_t id, uint32_t ch,
                               struct csi_ch_params *param);
    int (*channel_cfg_size)(uint32_t id, uint32_t ch,
                            struct csi_ch_params *param);
    int (*channel_cfg_crop)(uint32_t id, uint32_t ch,
                            struct csi_ch_params *param);
    int (*channel_enum_pixfmt)(uint32_t id, uint32_t ch,
                               struct csi_ch_params *param);
    int (*channel_cfg_pixfmt)(uint32_t id, uint32_t ch,
                              struct csi_ch_params *param);
    int (*channel_cfg_memrange)(uint32_t id, uint32_t ch, uint32_t low[3],
                                uint32_t up[3]);
};

struct csi_device {
    struct csi_channel_ops ops;
};

struct csi_device *camera_dev_init(uint32_t id);

#endif /* __CSI_DRV_H__ */
