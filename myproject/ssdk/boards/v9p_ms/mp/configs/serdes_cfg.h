/**
 * @file serdes_cfg.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: serdes config header file
 *
 * Revision History:
 * -----------------
 * */

#ifndef INCLUDE_SERDES_CFG_H
#define INCLUDE_SERDES_CFG_H

#ifdef CONFIG_SERDES
#include <types.h>

#include <driver/kunlun/serdes/serdes.h>
#include <driver/kunlun/serdes/serdes_vendor.h>

typedef enum {
    SERDES_DEV_DSI1,
    SERDES_DEV_DSI2,
    SERDES_DEV_LVDS4,

    SERDES_DEV_MAX,
}serdes_dev_enum_t;

extern serdes_dev_t* g_serdes_dev[SERDES_DEV_MAX];

extern int32_t board_serdes_init();

#endif /* end CONFIG_SERDES */
#endif /* INCLUDE_SERDES_CFG_H */
/* end of header file serdes_cfg.h */