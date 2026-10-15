/**
 * @file pvt_cfg.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: pvt config header file
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_PVT_CFG_H
#define INCLUDE_PVT_CFG_H

#ifdef CONFIG_PVT

#include <driver/kunlun/pvt/pvt.h>
#include <driver/kunlun/pvt/sdrv_pvt_drv.h>

typedef enum {
    PVT_DEV_SAFETY_INS,
    PVT_DEV_AP_INS,

    PVT_DEV_INS_NUM,
}pvt_dev_ins_type_enum_t;

extern pvt_dev_t* g_pvt_dev_ptr[PVT_DEV_INS_NUM];

int32_t board_pvt_init();
#endif /* end CONFIG_PVT */

#endif /* INCLUDE_PVT_CFG_H */
/* end of header file pvt_cfg.h */