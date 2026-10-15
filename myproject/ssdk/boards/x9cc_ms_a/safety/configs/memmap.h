/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
#ifndef BOARDS_MEMMAP_H
#define BOARDS_MEMMAP_H

#define APB_MUX_PHY_BASE        0xf0000000
#define APB_MUX_PHY_BASE_AP     0x60000000
#define APB_MUX_SPACE_SIZE      0x10000000

#define DDR_PHY_BASE            0x70000000
#define DDR_PHY_BASE_AP         0x80000000
#define DDR_SPACE_SIZE          0x80000000

#define MEMMAP_TABLE \
    {/*DDR */\
        DDR_PHY_BASE,\
        DDR_SPACE_SIZE,\
        DDR_PHY_BASE_AP,\
    },\
    {/*APB_MUX */\
        APB_MUX_PHY_BASE,\
        APB_MUX_SPACE_SIZE,\
        APB_MUX_PHY_BASE_AP,\
    },

#endif
