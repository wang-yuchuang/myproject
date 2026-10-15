//*****************************************************************************
//
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_CAN

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

static flexcan_mb_config_t g_flexcan_mb_cfg[] = {
    
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
    #ifdef CONFIG_CAN5
    {
        .maxMBNum = 14,
        .rx_mb_count = 7,
        .tx_mb_count = 7,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN6
    {
        .maxMBNum = 14,
        .rx_mb_count = 7,
        .tx_mb_count = 7,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN7
    {
        .maxMBNum = 14,
        .rx_mb_count = 7,
        .tx_mb_count = 7,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN8
    {
        .maxMBNum = 14,
        .rx_mb_count = 7,
        .tx_mb_count = 7,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN9
    {
        .maxMBNum = 14,
        .rx_mb_count = 7,
        .tx_mb_count = 7,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN10
    {
        .maxMBNum = 14,
        .rx_mb_count = 7,
        .tx_mb_count = 7,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN11
    {
        .maxMBNum = 14,
        .rx_mb_count = 7,
        .tx_mb_count = 7,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN12
    {
        .maxMBNum = 14,
        .rx_mb_count = 7,
        .tx_mb_count = 7,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN13
    {
        .maxMBNum = 14,
        .rx_mb_count = 7,
        .tx_mb_count = 7,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN14
    {
        .maxMBNum = 14,
        .rx_mb_count = 7,
        .tx_mb_count = 7,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN15
    {
        .maxMBNum = 14,
        .rx_mb_count = 7,
        .tx_mb_count = 7,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN16
    {
        .maxMBNum = 14,
        .rx_mb_count = 7,
        .tx_mb_count = 7,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN17
    {
        .maxMBNum = 14,
        .rx_mb_count = 7,
        .tx_mb_count = 7,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN18
    {
        .maxMBNum = 14,
        .rx_mb_count = 7,
        .tx_mb_count = 7,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN19
    {
        .maxMBNum = 14,
        .rx_mb_count = 7,
        .tx_mb_count = 7,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN20
    {
        .maxMBNum = 14,
        .rx_mb_count = 7,
        .tx_mb_count = 7,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN21
    {
        .maxMBNum = 14,
        .rx_mb_count = 7,
        .tx_mb_count = 7,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN22
    {
        .maxMBNum = 14,
        .rx_mb_count = 7,
        .tx_mb_count = 7,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN23
    {
        .maxMBNum = 14,
        .rx_mb_count = 7,
        .tx_mb_count = 7,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
    #ifdef CONFIG_CAN24
    {
        .maxMBNum = 14,
        .rx_mb_count = 7,
        .tx_mb_count = 7,
        .rx_mb_cfg = &flexcan_rx_mb_cfg[0],
        .tx_mb_cfg = &flexcan_tx_mb_cfg[0]
    },
    #endif
};

static flexcan_controller_cfg_t g_flexcan_cfg[] = {
    #ifdef CONFIG_CAN1
    {
        .channel = 0,
        .base = APB_CANFD1_BASE,
        .irq = CANFD1_CANFD_INTR_NUM,
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
        .base = APB_CANFD2_BASE,
        .irq = CANFD2_CANFD_INTR_NUM,
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
        .base = APB_CANFD3_BASE,
        .irq = CANFD3_CANFD_INTR_NUM,
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
        .base = APB_CANFD4_BASE,
        .irq = CANFD4_CANFD_INTR_NUM,
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
    #ifdef CONFIG_CAN5
    {
        .channel = 4,
        .base = APB_CANFD5_BASE,
        .irq = CANFD5_CANFD_INTR_NUM,
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
    #ifdef CONFIG_CAN6
    {
        .channel = 5,
        .base = APB_CANFD6_BASE,
        .irq = CANFD6_CANFD_INTR_NUM,
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
    #ifdef CONFIG_CAN7
    {
        .channel = 6,
        .base = APB_CANFD7_BASE,
        .irq = CANFD7_CANFD_INTR_NUM,
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
    #ifdef CONFIG_CAN8
    {
        .channel = 7,
        .base = APB_CANFD8_BASE,
        .irq = CANFD8_CANFD_INTR_NUM,
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
    #ifdef CONFIG_CAN9
    {
        .channel = 8,
        .base = APB_CANFD9_BASE,
        .irq = CANFD9_CANFD_INTR_NUM,
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
    #ifdef CONFIG_CAN10
    {
        .channel = 9,
        .base = APB_CANFD10_BASE,
        .irq = CANFD10_CANFD_INTR_NUM,
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
    #ifdef CONFIG_CAN11
    {
        .channel = 10,
        .base = APB_CANFD11_BASE,
        .irq = CANFD11_CANFD_INTR_NUM,
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
    #ifdef CONFIG_CAN12
    {
        .channel = 11,
        .base = APB_CANFD12_BASE,
        .irq = CANFD12_CANFD_INTR_NUM,
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
    #ifdef CONFIG_CAN13
    {
        .channel = 12,
        .base = APB_CANFD13_BASE,
        .irq = CANFD13_CANFD_INTR_NUM,
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
    #ifdef CONFIG_CAN14
    {
        .channel = 13,
        .base = APB_CANFD14_BASE,
        .irq = CANFD14_CANFD_INTR_NUM,
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
    #ifdef CONFIG_CAN15
    {
        .channel = 14,
        .base = APB_CANFD15_BASE,
        .irq = CANFD15_CANFD_INTR_NUM,
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
    #ifdef CONFIG_CAN16
    {
        .channel = 15,
        .base = APB_CANFD16_BASE,
        .irq = CANFD16_CANFD_INTR_NUM,
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
    #ifdef CONFIG_CAN17
    {
        .channel = 16,
        .base = APB_CANFD17_BASE,
        .irq = CANFD17_CANFD_INTR_NUM,
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
    #ifdef CONFIG_CAN18
    {
        .channel = 17,
        .base = APB_CANFD18_BASE,
        .irq = CANFD18_CANFD_INTR_NUM,
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
    #ifdef CONFIG_CAN19
    {
        .channel = 18,
        .base = APB_CANFD19_BASE,
        .irq = CANFD19_CANFD_INTR_NUM,
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
    #ifdef CONFIG_CAN20
    {
        .channel = 19,
        .base = APB_CANFD20_BASE,
        .irq = CANFD20_CANFD_INTR_NUM,
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
    #ifdef CONFIG_CAN21
    {
        .channel = 20,
        .base = APB_CANFD21_BASE,
        .irq = CANFD21_CANFD_INTR_NUM,
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
    #ifdef CONFIG_CAN22
    {
        .channel = 21,
        .base = APB_CANFD22_BASE,
        .irq = CANFD22_CANFD_INTR_NUM,
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
    #ifdef CONFIG_CAN23
    {
        .channel = 22,
        .base = APB_CANFD23_BASE,
        .irq = CANFD23_CANFD_INTR_NUM,
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
    #ifdef CONFIG_CAN24
    {
        .channel = 23,
        .base = APB_CANFD24_BASE,
        .irq = CANFD24_CANFD_INTR_NUM,
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