/*
* driver/taishan/g2dlite/g2dlite_log.h
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
#ifndef G2DLITE_LOG_H__
#define G2DLITE_LOG_H__
#include <debug.h>

#define G2DLITE_LOG_ENABLE  1

#define G2DLITE_LOG_LEVEL_EMERG  0 /*System is unusable*/
#define G2DLITE_LOG_LEVEL_ALERT  1 /*Action must be taken immediately*/
#define G2DLITE_LOG_LEVEL_CRIT   2 /*Critical conditions*/
#define G2DLITE_LOG_LEVEL_ERR    3 /*Error conditions*/
#define G2DLITE_LOG_LEVEL_WARN   4 /*Warning conditions*/
#define G2DLITE_LOG_LEVEL_NOTICE 5 /*Normal, but significant, conditions*/
#define G2DLITE_LOG_LEVEL_INFO   6 /*Informational message*/
#define G2DLITE_LOG_LEVEL_DEBUG  7 /*Debug-level message*/

/*log level default*/
#define G2DLITE_LOG_LEVEL   G2DLITE_LOG_LEVEL_ERR


#define G2DLITE_LOG_EMERG(string, args...) \
    if (G2DLITE_LOG_ENABLE && (G2DLITE_LOG_LEVEL >=G2DLITE_LOG_LEVEL_EMERG)) { \
        printf("[G2DLITE]EMERG|%s| "string, __func__, ##args); \
    }

#define G2DLITE_LOG_ALERT(string, args...) \
    if (G2DLITE_LOG_ENABLE && (G2DLITE_LOG_LEVEL >=G2DLITE_LOG_LEVEL_ALERT)) { \
        printf("[G2DLITE]ALERT|%s| "string, __func__, ##args); \
    }

#define G2DLITE_LOG_CRIT(string, args...) \
    if (G2DLITE_LOG_ENABLE && (G2DLITE_LOG_LEVEL >=G2DLITE_LOG_LEVEL_CRIT)) { \
        printf("[G2DLITE]CRIT|%s| "string, __func__, ##args); \
    }

#define G2DLITE_LOG_ERR(string, args...) \
    if (G2DLITE_LOG_ENABLE && (G2DLITE_LOG_LEVEL >= G2DLITE_LOG_LEVEL_ERR)) { \
        printf("[G2DLITE]ERR|%s| "string, __func__, ##args); \
    }

#define G2DLITE_LOG_WARN(string, args...) \
    if (G2DLITE_LOG_ENABLE && (G2DLITE_LOG_LEVEL >= G2DLITE_LOG_LEVEL_WARN)) { \
        printf("[G2DLITE]WARN|%s| "string, __func__, ##args); \
    }

#define G2DLITE_LOG_NOTICE(string, args...) \
    if (G2DLITE_LOG_ENABLE && (G2DLITE_LOG_LEVEL >= G2DLITE_LOG_LEVEL_NOTICE)) { \
        printf("[G2DLITE]NOTICE|%s| "string, __func__, ##args); \
    }

#define G2DLITE_LOG_INFO(string, args...) \
    if (G2DLITE_LOG_ENABLE && (G2DLITE_LOG_LEVEL >= G2DLITE_LOG_LEVEL_INFO)) { \
        printf("[G2DLITE]INFO|%s| "string, __func__, ##args); \
    }

#define G2DLITE_LOG_DEBUG(string, args...) \
    if (G2DLITE_LOG_ENABLE && (G2DLITE_LOG_LEVEL >= G2DLITE_LOG_LEVEL_DEBUG)) { \
        printf("[G2DLITE]DEBUG|%s| "string, __func__, ##args); \
    }

#define G2DLITE_LOG_FUNC() \
    if (G2DLITE_LOG_ENABLE && (G2DLITE_LOG_LEVEL >= G2DLITE_LOG_LEVEL_INFO)) { \
        printf("[G2DLITE]FUNC|%s\n", __func__); \
    }

#endif //_G2DLITE_DRV_LOG_H__

