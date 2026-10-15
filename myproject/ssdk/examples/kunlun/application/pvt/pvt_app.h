/**
 * @file pvt_app.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef __INCLUDE_PVT_APP_H__
#define __INCLUDE_PVT_APP_H__
/* =========================== include file session ======================== */
#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */
/**
* @defgroup XXX XXX
* @brief  XXX service
* @{
*/

/* =========================== marco define session ======================== */
#define PVT_MON_ALARM_SET   0       /**< notice alarm happen  */
#define PVT_MON_ALARM_CLR   1       /**< notice alarm release */
/* =========================== enumeration define session ================== */
/* =========================== structure define session ==================== */

/* =========================== type define session ========================= */
/**
 * @brief
 *
 * @param monIns[IN] monitor ins 0/1
 * @param type[IN] notify type, 0: alarm; 1: alarm release
 * @param args[IN] args
 * @return ** void
 */
typedef void (*cb_t)(int32_t monIns, int32_t type, void* args);

/* =========================== function declare session ==================== */
int32_t pvt_app_init();

int32_t pvt_app_deinit();


#ifdef CONFIG_PVT_APPLICATION_PRINT
int32_t pvt_app_print_init();
#endif /* end CONFIG_PVT_APPLICATION_PRINT */


#ifdef CONFIG_PVT_APPLICATION_MONITOR
int32_t pvt_monitor_init();
int32_t pvt_monitor_deinit();
int32_t pvt_monitor_register_noticer(int32_t monIns, cb_t cb, void* args);
#endif /* end CONFIG_PVT_APPLICATION_MONITOR */

/** @} */
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __INCLUDE_PVT_APP_H__ */
/* end of header file pvt_app.h */