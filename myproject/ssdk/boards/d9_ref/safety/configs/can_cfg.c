//*****************************************************************************
//
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_CAN

#include <stdio.h>
#include <debug.h>

#include "can_cfg.h"
#include "regs_base.h"
#include "irq_num.h"
#include <param.h>

static rx_mb_cfg_t flexcan_rx_mb_cfg[7] = {
    /* ID, MASK, RTR, STD */
    {0x0, 0x0, false, false},
    {0x0, 0x0, false, false},
    {0x0, 0x0, false, false},
    {0x0, 0x0, false, false},
    {0x0, 0x0, false, false},
    {0x0, 0x0, false, true},
    {0x0, 0x0, false, true}
};

static tx_mb_cfg_t flexcan_tx_mb_cfg[7] = {
    /* padding val */
    { 0 },
    { 0 },
    { 0 },
    { 0 },
    { 0 },
    { 0 },
    { 0 },
};

static flexcan_mb_config_t g_flexcan_mb_cfg[CAN_NUM_ON_BOARD] = {
    #ifdef CONFIG_CAN1
    {
        .maxMBNum = 14,
        .rx_mb_count = 7,
        .tx_mb_count = 7,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN2
    {
        .maxMBNum = 14,
        .rx_mb_count = 7,
        .tx_mb_count = 7,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN3
    {
        .maxMBNum = 14,
        .rx_mb_count = 7,
        .tx_mb_count = 7,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN4
    {
        .maxMBNum = 14,
        .rx_mb_count = 7,
        .tx_mb_count = 7,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
};

static flexcan_controller_cfg_t g_flexcan_cfg[CAN_NUM_ON_BOARD] = {
    #ifdef CONFIG_CAN1
    {
        .channel = 0,
        .base = APB_CAN1_BASE,
        .irq = CANFD1_IPI_BUSOFF_INTR_NUM,
        .is_canfd_enabled = true,
        .enableSelfWakeup = true,
        .enableIndividMask = true,
        BAUDRATE_1M_5M,
        .buf_cfg = {
            .txfifo_size = 30,
            .rxfifo_size = 30
        }
    },
    #endif
    #ifdef CONFIG_CAN2
    {
        .channel = 1,
        .base = APB_CAN2_BASE,
        .irq = CANFD2_IPI_BUSOFF_INTR_NUM,
        .is_canfd_enabled = true,
        .enableSelfWakeup = true,
        .enableIndividMask = true,
        BAUDRATE_1M_5M,
        .buf_cfg = {
            .txfifo_size = 30,
            .rxfifo_size = 30
        }
    },
    #endif
    #ifdef CONFIG_CAN3
    {
        .channel = 2,
        .base = APB_CAN3_BASE,
        .irq = CANFD3_IPI_BUSOFF_INTR_NUM,
        .is_canfd_enabled = true,
        .enableSelfWakeup = true,
        .enableIndividMask = true,
        BAUDRATE_1M_5M,
        .buf_cfg = {
            .txfifo_size = 30,
            .rxfifo_size = 30
        }
    },
    #endif
    #ifdef CONFIG_CAN4
    {
        .channel = 3,
        .base = APB_CAN4_BASE,
        .irq = CANFD4_IPI_BUSOFF_INTR_NUM,
        .is_canfd_enabled = true,
        .enableSelfWakeup = true,
        .enableIndividMask = true,
        BAUDRATE_1M_5M,
        .buf_cfg = {
            .txfifo_size = 30,
            .rxfifo_size = 30
        }
    },
    #endif
};

void board_can_init(void)
{
    for(int i = 0; i < ARRAY_SIZE(g_flexcan_cfg); i++) {
        flexcan_probe(&g_flexcan_cfg[i], &g_flexcan_mb_cfg[i]);
    }
    
}

#endif