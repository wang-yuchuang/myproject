
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_ASW

#include "asw_cfg.h"

#include <types.h>
#include <board.h>

/* asw device driver headers */
#include <driver/taishan/asw/asw.h>

struct asw_dev aswdev = {0};

/* asw config structures */
static const struct asw_config asw_cfg = {
    .base = DEVICE_BASE(GAMA1),
    .irq = DEVICE_INTR(GAMA1_O_GAMA),
};

/* asw initialization */
void board_asw_init(void)
{
    sdrv_asw_probe(&aswdev, &asw_cfg);
}
#endif
