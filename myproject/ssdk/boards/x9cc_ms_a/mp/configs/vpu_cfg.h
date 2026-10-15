/**
 * @file vpu_cfg.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: vpu config
 *
 * Revision History:
 * -----------------
 */
#ifndef __VPU_CFG_H__
#define __VPU_CFG_H__
#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */
/**
* @defgroup XXX XXX
* @brief  XXX service
* @{
*/
#ifdef CONFIG_VPU
/* =========================== include file session ======================== */
#include <driver/kunlun/vpu/vpu.h>
/* =========================== enumeration define session ================== */
/* =========================== structure define session ==================== */
/* =========================== function declare session ==================== */
/* =========================== global value define session ================= */
extern vpu_dev_t* jpu_dev;
/* =========================== static value define session ================= */
/* =========================== function implementation session ============= */
extern void board_vpu_init();

#endif /* end CONFIG_VPU */
/** @} */
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __VPU_CFG_H__ */
/* end of header file vpu_cfg.h */