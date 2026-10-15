//*****************************************************************************
//
// disp_debug.h - Prototypes for the display debug
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __DISP_DEBUG_H__
#define __DISP_DEBUG_H__

#include <stdio.h>
#include <stdint.h>
#include <debug.h>
#include <cmsis_os2.h>
#include <board.h>

#define DISP_DEBUG 1

#ifndef mdelay
    #define mdelay(x) osDelay(x * 1000)
#endif

#define PRINT(level, fmt, args...) ssdk_printf(level, "disp: [%20s]: " fmt, __FUNCTION__, ##args)
#define LOGD(fmt, args...) PRINT(DISP_DEBUG, fmt, ##args)
#define LOGE(fmt, args...) PRINT(0, "Error " fmt, ##args)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#endif // __DISP_DEBUG_H__

