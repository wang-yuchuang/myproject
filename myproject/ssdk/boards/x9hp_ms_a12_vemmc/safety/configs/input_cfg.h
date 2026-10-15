/**
 * @file input_cfg.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: touch service config
 *
 * Revision History:
 * -----------------
 */
#ifndef __INPUT_CFG_H__
#define __INPUT_CFG_H__
/* =========================== include file session ======================== */
#ifdef CONFIG_INPUT_SERVICE
#include <input_service.h>
#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */
/**
* @defgroup XXX XXX
* @brief  XXX service
* @{
*/
/* =========================== marco define session ======================== */
/* =========================== enumeration define session ================== */
/* =========================== structure define session ==================== */
/* =========================== function declare session ==================== */
/* =========================== global value define session ================= */
#define TS_REPORT_CFG_TBL_SIZE  4
extern ts_report_config_t g_ts_report_cfg_tbl[TS_REPORT_CFG_TBL_SIZE];
/* =========================== static value define session ================= */
/* =========================== function implementation session ============= */

/** @} */
#ifdef __cplusplus
}

#endif /* end CONFIG_INPUT_SERVICE */

#endif /* __cplusplus */
#endif /* __INPUT_CFG_H__ */
/* end of header file input_cfg.h */