/*
* dispss_log.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 11/24/2021 BILLY create this file
*/
#ifndef DISPSS_LOG_H__
#define DISPSS_LOG_H__
#include <debug.h>

#define DISPSS_LOG_ENABLE  1

#define DISPSS_LOG_LEVEL_EMERG  0 /*System is unusable*/
#define DISPSS_LOG_LEVEL_ALERT  1 /*Action must be taken immediately*/
#define DISPSS_LOG_LEVEL_CRIT   2 /*Critical conditions*/
#define DISPSS_LOG_LEVEL_ERR    3 /*Error conditions*/
#define DISPSS_LOG_LEVEL_WARN   4 /*Warning conditions*/
#define DISPSS_LOG_LEVEL_NOTICE 5 /*Normal, but significant, conditions*/
#define DISPSS_LOG_LEVEL_INFO   6 /*Informational message*/
#define DISPSS_LOG_LEVEL_DEBUG  7 /*Debug-level message*/

/*log level default*/
#define DISPSS_LOG_LEVEL    CONFIG_DEBUG_LEVEL


#define DISPSS_LOG_EMERG(string, args...) \
    if (DISPSS_LOG_ENABLE && (DISPSS_LOG_LEVEL >=DISPSS_LOG_LEVEL_EMERG)) { \
        printf("[DISPSS]EMERG|%s| "string, __func__, ##args); \
    }

#define DISPSS_LOG_ALERT(string, args...) \
    if (DISPSS_LOG_ENABLE && (DISPSS_LOG_LEVEL >=DISPSS_LOG_LEVEL_ALERT)) { \
        printf("[DISPSS]ALERT|%s| "string, __func__, ##args); \
    }

#define DISPSS_LOG_CRIT(string, args...) \
    if (DISPSS_LOG_ENABLE && (DISPSS_LOG_LEVEL >=DISPSS_LOG_LEVEL_CRIT)) { \
        printf("[DISPSS]CRIT|%s| "string, __func__, ##args); \
    }

#define DISPSS_LOG_ERR(string, args...) \
    if (DISPSS_LOG_ENABLE && (DISPSS_LOG_LEVEL >= DISPSS_LOG_LEVEL_ERR)) { \
        printf("[DISPSS]ERR|%s| "string, __func__, ##args); \
    }

#define DISPSS_LOG_WARN(string, args...) \
    if (DISPSS_LOG_ENABLE && (DISPSS_LOG_LEVEL >= DISPSS_LOG_LEVEL_WARN)) { \
        printf("[DISPSS]WARN|%s| "string, __func__, ##args); \
    }

#define DISPSS_LOG_NOTICE(string, args...) \
    if (DISPSS_LOG_ENABLE && (DISPSS_LOG_LEVEL >= DISPSS_LOG_LEVEL_NOTICE)) { \
        printf("[DISPSS]NOTICE|%s| "string, __func__, ##args); \
    }

#define DISPSS_LOG_INFO(string, args...) \
    if (DISPSS_LOG_ENABLE && (DISPSS_LOG_LEVEL >= DISPSS_LOG_LEVEL_INFO)) { \
        printf("[DISPSS]INFO|%s| "string, __func__, ##args); \
    }

#define DISPSS_LOG_DEBUG(string, args...) \
    if (DISPSS_LOG_ENABLE && (DISPSS_LOG_LEVEL >= DISPSS_LOG_LEVEL_DEBUG)) { \
        printf("[DISPSS]DEBUG|%s| "string, __func__, ##args); \
    }

#define DISPSS_LOG_FUNC() \
    if (DISPSS_LOG_ENABLE && (DISPSS_LOG_LEVEL >= DISPSS_LOG_LEVEL_INFO)) { \
        printf("[DISPSS]FUNC|%s\n", __func__); \
    }

#endif //_DISPSS_DRV_LOG_H__

