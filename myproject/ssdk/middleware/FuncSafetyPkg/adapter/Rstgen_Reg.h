/**
*   @file    Rstgen_Reg.h
*
*   @brief   Register definitions of Rstgen HW IP.
*/

/********************************************************
 *        Copyright(c) 2020    Semidrive                 *
 *        All rights reserved.                           *
 ********************************************************/

#ifndef RSTGEN_REG_H
#define RSTGEN_REG_H

#ifdef __cplusplus
extern "C" {
#endif
/* Version and Check Begin */
/* Version Info Begin */
#define RSTGEN_REG_H_VENDOR_ID    0x8C
#define RSTGEN_REG_H_AR_RELEASE_MAJOR_VERSION    4
#define RSTGEN_REG_H_AR_RELEASE_MINOR_VERSION    3
#define RSTGEN_REG_H_AR_RELEASE_REVISION_VERSION 1
#define RSTGEN_REG_H_SW_MAJOR_VERSION    1
#define RSTGEN_REG_H_SW_MINOR_VERSION    0
#define RSTGEN_REG_H_SW_PATCH_VERSION    0
/* Version Info End */
/* Version and Check End */

#define RSTGEN_GLOBAL_RST_EN_OFF(n) (0x0UL + 4UL*((uint32)n))

#define BM_RSTGEN_GLOBAL_RST_EN_GLB_RST_LOCK    (0x01U << 31U)

#define FM_RSTGEN_GLOBAL_RST_EN_GLB_RST_EN  (0x7ffU << 0U)
#define FV_RSTGEN_GLOBAL_RST_EN_GLB_RST_EN(v) \
    (((v) << 0U) & FM_RSTGEN_GLOBAL_RST_EN_GLB_RST_EN)
#define GFV_RSTGEN_GLOBAL_RST_EN_GLB_RST_EN(v) \
    (((v) & FM_RSTGEN_GLOBAL_RST_EN_GLB_RST_EN) >> 0U)

#define RSTGEN_CORE_RST_EN_OFF(n)   (0x4U + 8U*(n))

#define BM_RSTGEN_CORE_RST_EN_CORE_RST_LOCK (0x01UL << 31U)

#define BM_RSTGEN_CORE_RST_EN_SW_RST_EN_STA (0x01UL << 30U)

#define BM_RSTGEN_CORE_RST_EN_WDT_RST_EN    (0x01U << 2U)

#define BM_RSTGEN_CORE_RST_EN_DBG_RST_EN    (0x01U << 1U)

#define BM_RSTGEN_CORE_RST_EN_SW_RST_EN (0x01U << 0U)

#define RSTGEN_CORE_SW_RST_OFF(n)   (0x8U + 8U*(n))

#define BM_RSTGEN_CORE_SW_RST_CORE_RST_STA  (0x01UL << 30U)

#define BM_RSTGEN_CORE_SW_RST_STATIC_RST_B_STA  (0x01UL << 29U)

#define BM_RSTGEN_CORE_SW_RST_AUTO_CLR_RST  (0x01U << 1U)

#define BM_RSTGEN_CORE_SW_RST_STATIC_RST_B  (0x01U << 0U)

#define RSTGEN_SW_SELF_RST_OFF  0x44U

#define BM_RSTGEN_SW_SELF_RST_SW_GLB_RST_LOCK   (0x01UL << 31U)

#define BM_RSTGEN_SW_SELF_RST_SW_GLB_RST    (0x01U << 0U)

#define RSTGEN_SW_OTH_RST_OFF   0x48U

#define BM_RSTGEN_SW_OTH_RST_SW_GLB_RST_LOCK    (0x01UL << 31U)

#define BM_RSTGEN_SW_OTH_RST_SW_GLB_RST (0x01U << 0U)

#define RSTGEN_RST_STA_OFF  0x4cU

#define FM_RSTGEN_RST_STA_RST_STA   (0xffffffffU << 0U)
#define FV_RSTGEN_RST_STA_RST_STA(v) \
    (((v) << 0U) & FM_RSTGEN_RST_STA_RST_STA)
#define GFV_RSTGEN_RST_STA_RST_STA(v) \
    (((v) & FM_RSTGEN_RST_STA_RST_STA) >> 0U)

#define RSTGEN_GENERAL_REG_OFF(n)   (0x50U + 4U*(n))

#define FM_RSTGEN_GENERAL_REG_GENERAL_REG   (0xffffffffU << 0U)
#define FV_RSTGEN_GENERAL_REG_GENERAL_REG(v) \
    (((v) << 0U) & FM_RSTGEN_GENERAL_REG_GENERAL_REG)
#define GFV_RSTGEN_GENERAL_REG_GENERAL_REG(v) \
    (((v) & FM_RSTGEN_GENERAL_REG_GENERAL_REG) >> 0U)

#define RSTGEN_ISO_EN_OFF(n)    (0x90U + 4U*((uint32)n))

#define BM_RSTGEN_ISO_EN_ISO_B  (0x01U << 0U)

#define RSTGEN_MODULE_RST_OFF(n)    (0x100U + 4U*(n))

#define BM_RSTGEN_MODULE_RST_MODULE_RST_LOCK    (0x01UL << 31U)

#define BM_RSTGEN_MODULE_RST_MODULE_RST_STA (0x01UL << 30U)

#define BM_RSTGEN_MODULE_RST_MODULE_RST_EN  (0x01U << 1U)

#define BM_RSTGEN_MODULE_RST_MODULE_RST_N   (0x01U << 0U)

#ifdef __cplusplus
}
#endif

#endif  /* ifdef RSTGEN_REG_H */
