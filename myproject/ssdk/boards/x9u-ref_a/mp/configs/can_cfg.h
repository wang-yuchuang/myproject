//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#ifndef __CAN_RES__
#define __CAN_RES__
#ifdef CONFIG_CAN
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

extern void board_vcan_init(void);
#endif /* end CONFIG_CAN */
#endif /*__CAN_RES__*/