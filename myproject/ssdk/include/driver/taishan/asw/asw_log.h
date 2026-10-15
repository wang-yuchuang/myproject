/*
* asw_log.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 12/08/2021 BILLY create this file
*/
#ifndef ASW_LOG_H__
#define ASW_LOG_H__
#include <debug.h>

#define ASW_LOG_ENABLE  1

#define ASW_LOG_LEVEL_EMERG  0 /*System is unusable*/
#define ASW_LOG_LEVEL_ALERT  1 /*Action must be taken immediately*/
#define ASW_LOG_LEVEL_CRIT   2 /*Critical conditions*/
#define ASW_LOG_LEVEL_ERR    3 /*Error conditions*/
#define ASW_LOG_LEVEL_WARN   4 /*Warning conditions*/
#define ASW_LOG_LEVEL_NOTICE 5 /*Normal, but significant, conditions*/
#define ASW_LOG_LEVEL_INFO   6 /*Informational message*/
#define ASW_LOG_LEVEL_DEBUG  7 /*Debug-level message*/

/*log level default*/
#define ASW_LOG_LEVEL   ASW_LOG_LEVEL_ERR


#define ASW_LOG_EMERG(string, args...) \
    if (ASW_LOG_ENABLE && (ASW_LOG_LEVEL >=ASW_LOG_LEVEL_EMERG)) { \
        printf("[ASW]EMERG|%s| "string, __func__, ##args); \
    }

#define ASW_LOG_ALERT(string, args...) \
    if (ASW_LOG_ENABLE && (ASW_LOG_LEVEL >=ASW_LOG_LEVEL_ALERT)) { \
        printf("[ASW]ALERT|%s| "string, __func__, ##args); \
    }

#define ASW_LOG_CRIT(string, args...) \
    if (ASW_LOG_ENABLE && (ASW_LOG_LEVEL >=ASW_LOG_LEVEL_CRIT)) { \
        printf("[ASW]CRIT|%s| "string, __func__, ##args); \
    }

#define ASW_LOG_ERR(string, args...) \
    if (ASW_LOG_ENABLE && (ASW_LOG_LEVEL >= ASW_LOG_LEVEL_ERR)) { \
        printf("[ASW]ERR|%s| "string, __func__, ##args); \
    }

#define ASW_LOG_WARN(string, args...) \
    if (ASW_LOG_ENABLE && (ASW_LOG_LEVEL >= ASW_LOG_LEVEL_WARN)) { \
        printf("[ASW]WARN|%s| "string, __func__, ##args); \
    }

#define ASW_LOG_NOTICE(string, args...) \
    if (ASW_LOG_ENABLE && (ASW_LOG_LEVEL >= ASW_LOG_LEVEL_NOTICE)) { \
        printf("[ASW]NOTICE|%s| "string, __func__, ##args); \
    }

#define ASW_LOG_INFO(string, args...) \
    if (ASW_LOG_ENABLE && (ASW_LOG_LEVEL >= ASW_LOG_LEVEL_INFO)) { \
        printf("[ASW]INFO|%s| "string, __func__, ##args); \
    }

#define ASW_LOG_DEBUG(string, args...) \
    if (ASW_LOG_ENABLE && (ASW_LOG_LEVEL >= ASW_LOG_LEVEL_DEBUG)) { \
        printf("[ASW]DEBUG|%s| "string, __func__, ##args); \
    }

#define ASW_LOG_FUNC() \
    if (ASW_LOG_ENABLE && (ASW_LOG_LEVEL >= ASW_LOG_LEVEL_INFO)) { \
        printf("[ASW]FUNC|%s\n", __func__); \
    }

#endif //_ASW_DRV_LOG_H__

