
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#ifdef CONFIG_G2DLITE
#include "g2dlite_cfg.h"

#include <types.h>
#include <board.h>

struct g2dlite_device g2dlite_dev = {0};

/* g2dlite config structures */
static const struct g2dlite_config g2dlite_cfg = {
    .base = DEVICE_BASE(G2D2),
    .irq = DEVICE_INTR(G2D2_G2D),
};

/* g2dlite initialization */
void board_g2dlite_init(void)
{
    sdrv_g2dlite_probe(&g2dlite_dev, &g2dlite_cfg);
}
#endif
