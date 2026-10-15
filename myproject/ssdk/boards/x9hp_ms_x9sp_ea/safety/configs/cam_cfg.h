/**
 * @file cam_cfg.h
 * @brief external camera resource config header.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifdef CONFIG_CSI
#ifndef __CAM_CFG_H__
#define __CAM_CFG_H__

#include <driver/kunlun/csi/cam_res.h>

void board_camera_init(void);

#endif /* __CAM_CFG_H__ */
#endif