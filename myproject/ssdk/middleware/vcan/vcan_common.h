/*
 * vcan_common.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef VCAN_COMMON_H
#define VCAN_COMMON_H

#include <debug.h>

#define VCAN_DEBUG_LVL      SSDK_WARNING

#define EXTRA_FMT   "vcan:"

#if VCAN_DEBUG_LVL >= SSDK_EMERG
#define VCAN_ALWAYS(format, ...) \
    ssdk_printf(SSDK_EMERG, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define VCAN_ALWAYS(format, ...)
#endif

#if VCAN_DEBUG_LVL >= SSDK_ERR
#define VCAN_ERR(format, ...) \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define VCAN_ERR(format, ...)
#endif

#if VCAN_DEBUG_LVL >= SSDK_WARNING
#define VCAN_WARN(format, ...) \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define VCAN_WARN(format, ...)
#endif

#if VCAN_DEBUG_LVL >= SSDK_INFO
#define VCAN_INFO(format, ...) \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define VCAN_INFO(format, ...)
#endif

#endif
