/*
 * g2dlite_drv_log.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 * 011, 04/28/2020 BI create this file
 */
#ifndef _G2DLITE_DRV_LOG_H__
#define _G2DLITE_DRV_LOG_H__
#include <debug.h>

#define G2DLITE_LOG_ENABLE 1

#define G2DLITE_LOG_LEVEL_TRACE                                                \
    0 /**< A lot of logs to give detailed information*/
#define G2DLITE_LOG_LEVEL_INFO 1 /**< Log important events*/
#define G2DLITE_LOG_LEVEL_WARN                                                 \
    2 /**< Log if something unwanted happened but didn't caused problem*/
#define G2DLITE_LOG_LEVEL_ERROR                                                \
    3 /**< Only critical issue, when the system may fail*/
#define G2DLITE_LOG_LEVEL_NONE 4 /**< Do not log anything*/
#define _G2DLITE_LOG_LEVEL_NUM 5 /**< Number of log levels */

/*log level default*/
#define G2DLITE_LOG_LEVEL G2DLITE_LOG_LEVEL_ERROR

#define G2DLITE_LOG_TREACE(string, args...)                                    \
    if (G2DLITE_LOG_ENABLE &&                                                  \
        (G2DLITE_LOG_LEVEL <= G2DLITE_LOG_LEVEL_TRACE)) {                      \
        printf("[G2DLITE]TREACE|%s| " string, __func__, ##args);               \
    }

#define G2DLITE_LOG_INFO(string, args...)                                      \
    if (G2DLITE_LOG_ENABLE && (G2DLITE_LOG_LEVEL <= G2DLITE_LOG_LEVEL_INFO)) { \
        printf("[G2DLITE]INFO|%s| " string, __func__, ##args);                 \
    }

#define G2DLITE_LOG_FUNC()                                                     \
    if (G2DLITE_LOG_ENABLE && (G2DLITE_LOG_LEVEL <= G2DLITE_LOG_LEVEL_INFO)) { \
        printf("[G2DLITE]FUNC|%s\n", __func__);                                \
    }

#define G2DLITE_LOG_WARN(string, args...)                                      \
    if (G2DLITE_LOG_ENABLE && (G2DLITE_LOG_LEVEL <= G2DLITE_LOG_LEVEL_WARN)) { \
        printf("[G2DLITE]WARN|%s| " string, __func__, ##args);                 \
    }

#define G2DLITE_LOG_ERR(string, args...)                                       \
    if (G2DLITE_LOG_ENABLE &&                                                  \
        (G2DLITE_LOG_LEVEL <= G2DLITE_LOG_LEVEL_ERROR)) {                      \
        printf("[G2DLITE]ERR|%s| " string, __func__, ##args);                  \
    }

#endif //_G2DLITE_DRV_LOG_H__
