/**
 * @file sdrv_slimfs_log.h
 *
 * Copyright (c) 2023 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef __SDRV_SLIMFS_LOG_H__
#define __SDRV_SLIMFS_LOG_H__
/* =========================== include file session ======================== */
#include <string.h>
#include <stdio.h>
#include <debug.h>
#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */
/**
* @defgroup XXX XXX
* @brief  XXX service
* @{
*/
/* =========================== marco define session ======================== */
#define SLIMFS_LOG_ERR      0
#define SLIMFS_LOG_WARN     1
#define SLIMFS_LOG_INFO     2
#define SLIMFS_LOG_DEBUG    3

#define SLIMFS_LOG_LEV      SLIMFS_LOG_INFO
#define filename(x) strrchr(x, '/') ? strrchr(x, '/')+1 : x

#define SLIMFS_PRINT(type, format, ...) \
    do {\
        if (SLIMFS_LOG_LEV >= type) {ssdk_printf(SSDK_EMERG, "file=%s,line=%d "format"\n", filename(__FILE__), __LINE__, ##__VA_ARGS__);}\
    }while(0)

/* =========================== enumeration define session ================== */
/* =========================== structure define session ==================== */
/* =========================== function declare session ==================== */
/* =========================== global value define session ================= */
/* =========================== static value define session ================= */
/* =========================== function implementation session ============= */

/** @} */
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __SDRV_SLIMFS_LOG_H__ */
/* end of header file sdrv_slimfs_log.h */