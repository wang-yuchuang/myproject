//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef BOARDS_CAN_CFG_H
#define BOARDS_CAN_CFG_H

#ifdef CONFIG_CAN

#include <driver/can.h>
#include <driver/kunlun/can/flexcan_driver.h>

#ifndef CONFIG_CLOCK_OVER_DRIVE_MODE
/* for 80MHz  */
#define BAUDRATE_1M_5M  .nominal_bit_timing = {   /* 1Mbps, sample point 80% */ \
                            .preDivider = 2U, \
                            .rJumpwidth = 5U, \
                            .propSeg = 10U, \
                            .phaseSeg1 = 21U, \
                            .phaseSeg2 = 8U \
                        }, \
                        .data_bit_timing = {  /* 5Mbps, sample point 75% */ \
                            .preDivider = 2U, /* Should be the same as nominalBitTiming.preDivider. */ \
                            .rJumpwidth = 1U, \
                            .propSeg = 2U, \
                            .phaseSeg1 = 3U, \
                            .phaseSeg2 = 2U \
                        }, \
                        .fd_tdc = 12
#endif

#define CAN_NUM_ON_BOARD 4
#define CONFIG_CAN1
#define CONFIG_CAN2
#define CONFIG_CAN3
#define CONFIG_CAN4

/* can init functions */
extern void board_can_init(void);

#endif

#endif /* BOARDS_CAN_CFG_H */