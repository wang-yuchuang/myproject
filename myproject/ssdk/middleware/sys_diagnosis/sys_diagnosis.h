/**
 * @file sys_diagnosis.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef __SYS_DIAGNOSIS_H__
#define __SYS_DIAGNOSIS_H__
/* =========================== include file session ======================== */
#include <stdint.h>

#ifdef CONFIG_KUNLUN_SEM
#include <../drivers/kunlun/sem/kunlun_sem_lld.h>
#endif /* end CONFIG_KUNLUN_SEM */

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */
/**
* @defgroup XXX XXX
* @brief  XXX service
* @{
*/
/* =========================== marco define session ======================== */
#define NULL_SIGNAL -1

/* =========================== typedef define session ====================== */
typedef int (*sys_event_cb_t)(int signal, void* args);

/* =========================== function declare session ==================== */
/**
 * @brief register a callback to speicific signal(s)
 * e.g.
 * sysd_register_handler(callback,args,2,WDT1____ovflow_int,WDT2____ovflow_int);
 *
 * @param cb callback handle
 * @param args callback args
 * @param n sem signal number
 * @param ...
 * @return int
 * @retval 0 sucess
 * @retval <0 fail
 */
int sysd_register_handler(sys_event_cb_t cb, void* args, uint32_t n, ...);

/**
 * @brief register a daemon callback to speicific signal
 * This would be invoked at the last callback of the callback chain.
 *
 * @param cb callback handle
 * @param args callback args
 * @param signal sem signal
 * @return int
 * @retval 0 sucess
 * @retval <0 fail
 */
int sysd_register_daemon_handler(sys_event_cb_t cb,void* args,int signal);

/**
 * @brief start sys diagnosis service. Invoked at platform_init phase
 *
 * @param sem sem enum
 */
void sysd_start(enum sem sem);

/** @} */
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __SYS_DIAGNOSIS_H__ */
/* end of header file sys_diagnosis.h */