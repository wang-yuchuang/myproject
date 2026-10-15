/**
 * @file app_reboot_ss.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef __APP_REBOOT_SS_H__
#define __APP_REBOOT_SS_H__
/* =========================== include file session ======================== */
#include <stdint.h>
#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */
#ifdef CONFIG_KUNLUN_SEM
#include <../drivers/kunlun/sem/kunlun_sem_lld.h>
#endif /* end CONFIG_KUNLUN_SEM */
/**
* @defgroup XXX XXX
* @brief  XXX service
* @{
*/
/* =========================== function declare session ==================== */
extern int32_t app_reboot_ss_init(int sig);
/** @} */
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __APP_REBOOT_SS_H__ */
/* end of header file app_reboot_ss.h */
