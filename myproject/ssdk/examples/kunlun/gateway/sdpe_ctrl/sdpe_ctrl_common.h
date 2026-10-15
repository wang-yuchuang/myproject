/*
 * sdpe_ctrl_common.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef SDPE_CTRL_COMMON_H
#define SDPE_CTRL_COMMON_H

#include <debug.h>

#define SDPE_CTRL_DEBUG_LVL      SSDK_WARNING

#define EXTRA_FMT   "sdpe_ctrl:"

#if SDPE_CTRL_DEBUG_LVL >= SSDK_EMERG
#define SDPE_CTRL_ALWAYS(format, ...) \
    ssdk_printf(SSDK_EMERG, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDPE_CTRL_ALWAYS(format, ...)
#endif

#if SDPE_CTRL_DEBUG_LVL >= SSDK_ERR
#define SDPE_CTRL_ERR(format, ...) \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDPE_CTRL_ERR(format, ...)
#endif

#if SDPE_CTRL_DEBUG_LVL >= SSDK_WARNING
#define SDPE_CTRL_WARN(format, ...) \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDPE_CTRL_WARN(format, ...)
#endif

#if SDPE_CTRL_DEBUG_LVL >= SSDK_INFO
#define SDPE_CTRL_INFO(format, ...) \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDPE_CTRL_INFO(format, ...)
#endif

#endif
