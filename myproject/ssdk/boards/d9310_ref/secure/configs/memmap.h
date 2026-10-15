
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef BOARDS_MEMMAP_H
#define BOARDS_MEMMAP_H

#define APB_MUX_PHY_BASE        0xf0000000
#define APB_MUX_PHY_BASE_AP     0x30000000
#define APB_MUX_SPACE_SIZE      0x10000000

#define DDR_PHY_BASE            0x30000000
#define DDR_PHY_BASE_AP         0x40000000
#define DDR_SPACE_SIZE          0xc0000000

#define TCM_PHY_BASE            0x004b0000
#define TCM_PHY_BASE_AP         0x003b0000
#define TCM_SPACE_SIZE          0x50000

#define MEMMAP_TABLE \
    {/*TCM/CACHE */\
        TCM_PHY_BASE,\
        TCM_SPACE_SIZE,\
        TCM_PHY_BASE_AP,\
    },\
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
