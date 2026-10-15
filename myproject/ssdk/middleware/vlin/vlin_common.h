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

#define VLIN_DEBUG_LVL      SSDK_WARNING

#define EXTRA_FMT   "vlin:"

#if VLIN_DEBUG_LVL >= SSDK_EMERG
#define VLIN_ALWAYS(format, ...) \
    ssdk_printf(SSDK_EMERG, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define VLIN_ALWAYS(format, ...)
#endif

#if VLIN_DEBUG_LVL >= SSDK_ERR
#define VLIN_ERR(format, ...) \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define VLIN_ERR(format, ...)
#endif

#if VLIN_DEBUG_LVL >= SSDK_WARNING
#define VLIN_WARN(format, ...) \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define VLIN_WARN(format, ...)
#endif

#if VLIN_DEBUG_LVL >= SSDK_INFO
#define VLIN_INFO(format, ...) \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define VLIN_INFO(format, ...)
#endif

#endif
