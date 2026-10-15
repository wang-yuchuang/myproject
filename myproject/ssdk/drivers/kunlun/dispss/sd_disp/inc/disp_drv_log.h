/**
 * @file disp_drv_log.h
 * @brief display subsystem header file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef _DISP_DRV_LOG_H__
#define _DISP_DRV_LOG_H__
#include <driver/kunlun/dispss/dispss_log.h>

#include <debug.h>

#define DISP_DEBUG_BUILD 1

#if DISP_DEBUG_BUILD
#define DISP_DFINFO     (1)
#else
#define DISP_DFINFO     (0)
#endif

#if ENABLE_DISP_LINK
#define DISPMSG    DISPSS_LOG_EMERG
#define DISPDBG    DISPSS_LOG_DEBUG
#define DISPERR    DISPSS_LOG_ERR  //default on, err msg
#define DISPFUNC()

#define DISPCHECK(string, args...)
#else
#define DISPMSG    DISPSS_LOG_EMERG
#define DISPDBG    DISPSS_LOG_DEBUG
#define DISPERR    DISPSS_LOG_ERR  //default on, err msg

#define DISPFUNC()

#define DISPCHECK(string, args...)
#endif

#endif //_DISP_DRV_LOG_H__