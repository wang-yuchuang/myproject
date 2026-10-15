
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#include "dispss_cfg.h"

#include <types.h>
#include <board.h>

/* dc device driver headers */
#include <driver/taishan/dispss/disp.h>

/* dc device drivers */
struct dc_dev g_dc_dev;

/* dc config structures */
static const struct dc_config dc_cfg = {
    .base = DEVICE_BASE(DC),
    .irq = DEVICE_INTR(DC_DC_IRQ1),
};

/* dc initialization */
void board_dispss_init(void)
{
    sdrv_dc_probe(&g_dc_dev, &dc_cfg);
    sdrv_display_init(&g_dc_dev);
    sdrv_display_logo(&g_dc_dev, 0);
}

