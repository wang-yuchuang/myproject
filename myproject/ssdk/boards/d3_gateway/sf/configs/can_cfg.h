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
#include <driver/taishan/can/flexcan_driver.h>

#ifndef CONFIG_CLOCK_OVER_DRIVE_MODE
/* for 40MHz  */
#define BAUDRATE_1M_5M  .nominal_bit_timing = {   /* 1Mbps, sample point 80% */ \
                            .preDivider = 1U, \
                            .rJumpwidth = 5U, \
                            .propSeg = 10U, \
                            .phaseSeg1 = 21U, \
                            .phaseSeg2 = 8U \
                        }, \
                        .data_bit_timing = {  /* 5Mbps, sample point 75% */ \
                            .preDivider = 1U, /* Should be the same as nominalBitTiming.preDivider. */ \
                            .rJumpwidth = 1U, \
                            .propSeg = 2U, \
                            .phaseSeg1 = 3U, \
                            .phaseSeg2 = 2U \
                        }, \
                        .fd_tdc = 8
#endif

/* can init functions */
extern void board_can_init(void);

#endif

#endif /* BOARDS_CAN_CFG_H */