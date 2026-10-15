/**
 * @file sdrv_rstgen.h
 * @brief semidrive rstgen driver header.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef DRV_LLD_SDRV_RSTGEN_H
#define DRV_LLD_SDRV_RSTGEN_H

#include <types.h>
#include <compiler.h>

/* This section defines the register offsets of rstgen component */

#define GLB_RST_EN      (0x00u)
#if CONFIG_BOARD_X9SP
#define CORE_RST_EN     (0x380u)
#define CORE_SW_RST     (0x384u)
#else
#define CORE_RST_EN     (0x04u)
#define CORE_SW_RST     (0x08u)
#endif
#define SW_SELF_RST     (0x44u)
#define SW_OTH_RST      (0x48u)
#define RST_STA         (0x4Cu)
#define GENERAL_REG     (0x50u)
#define ISO_EN          (0x90u)
#define MODULE_RST      (0x100u)

/* SDRV_RSTGEN_GLB_RST_EN */
#define RSTGEN_GLB_RST_SELF_RST_EN     (0U)
#define RSTGEN_GLB_RST_SEM_RST_EN      (1U)
#define RSTGEN_GLB_RST_DBG_RST_EN      (2U)
#define RSTGEN_GLB_RST_WDG1_RST_EN     (3U)
#define RSTGEN_GLB_RST_WDG2_RST_EN     (4U)
#define RSTGEN_GLB_RST_WDG3_RST_EN     (5U)
#define RSTGEN_GLB_RST_WDG4_RST_EN     (6U)
#define RSTGEN_GLB_RST_WDG5_RST_EN     (7U)
#define RSTGEN_GLB_RST_WDG6_RST_EN     (8U)
#define RSTGEN_GLB_RST_WDG7_RST_EN     (9U)
#define RSTGEN_GLB_RST_WDG8_RST_EN     (10U)
#define RSTGEN_GLB_RST_LOCK            (31U)

/* SDRV_RSTGEN_CORE_RST_EN */
#define RSTGEN_CORE_RST_SW_RST_EN     (0U)
#define RSTGEN_CORE_RST_DBG_RST_EN    (1U)
#define RSTGEN_CORE_RST_WDG_RST_EN    (2U)
#define RSTGEN_CORE_RST_SW_RST_EN_STA (30U)
#define RSTGEN_CORE_RST_RST_LOCK      (31U)

/* SDRV_RSTGEN_CORE_SW_RST */
#define RSTGEN_CORE_SW_RST_STATIC_RST       (0U)
#define RSTGEN_CORE_SW_RST_AUTO_CLR         (1U)
#define RSTGEN_CORE_SW_RST_STATIC_RST_B_STA (29U)
#define RSTGEN_CORE_SW_RST_CORE_RST_STA     (30U)

/* SDRV_RSTGEN_SW_SELF_RST */
#define RSTGEN_SELF_RST_SW_GLB_RST       (0U)
#define RSTGEN_SELF_RST_SW_GLB_RST_LOCK  (31U)

/* SDRV_RSTGEN_SW_OTH_RST */
#define RSTGEN_OTH_RST_SW_GLB_RST        (0U)
#define RSTGEN_OTH_RST_SW_GLB_RST_LOCK   (31U)

/* SDRV_RSTGEN_RST_STA */
#define RSTGEN_RST_STA_PRE_SW_RST     (0U)
#define RSTGEN_RST_STA_SELF_SW_RST    (1U)
#define RSTGEN_RST_STA_SEM_RST        (2U)
#define RSTGEN_RST_STA_DBG_RST        (3U)
#define RSTGEN_RST_STA_WDG1_RST       (4U)
#define RSTGEN_RST_STA_WDG2_RST       (5U)
#define RSTGEN_RST_STA_WDG3_RST       (6U)
#define RSTGEN_RST_STA_WDG4_RST       (7U)
#define RSTGEN_RST_STA_WDG5_RST       (8U)
#define RSTGEN_RST_STA_WDG6_RST       (9U)
#define RSTGEN_RST_STA_WDG7_RST       (10U)
#define RSTGEN_RST_STA_WDG8_RST       (11U)
#define RSTGEN_RST_STA_SINC_PRE_SW_RST    (16U)
#define RSTGEN_RST_STA_SINC_SELF_SW_RST   (17U)
#define RSTGEN_RST_STA_SINC_SEM_RST       (18U)
#define RSTGEN_RST_STA_SINC_DBG_RST       (19U)
#define RSTGEN_RST_STA_SINC_WDG1_RST      (20U)
#define RSTGEN_RST_STA_SINC_WDG2_RST      (21U)
#define RSTGEN_RST_STA_SINC_WDG3_RST      (22U)
#define RSTGEN_RST_STA_SINC_WDG4_RST      (23U)
#define RSTGEN_RST_STA_SINC_WDG5_RST      (24U)
#define RSTGEN_RST_STA_SINC_WDG6_RST      (25U)
#define RSTGEN_RST_STA_SINC_WDG7_RST      (26U)
#define RSTGEN_RST_STA_SINC_WDG8_RST      (27U)

/* SDRV_RSTGEN_ISO_EN */
#define RSTGEN_ISO_EN_B       (0U)

/* SDRV_RSTGEN_MODULE_RST */
#define RSTGEN_MODULE_RST_N       (0U)
#define RSTGEN_MODULE_RST_EN      (1U)
#define RSTGEN_MODULE_RST_STA     (30U)
#define RSTGEN_MODULE_RST_LOCK    (31U)

/* offset */
#define RSTGEN_CORE_RST_EN_OFF(core_idx)    (CORE_RST_EN + 0x8*(core_idx))
#define RSTGEN_CORE_SW_RST_OFF(core_idx)    (CORE_SW_RST + 0x8*(core_idx))
#define RSTGEN_SW_SELF_RST_OFF              (SW_SELF_RST)
#define RSTGEN_SW_OTH_RST_OFF               (SW_OTH_RST)
#define RSTGEN_RST_STA_OFF                  (RST_STA)
#define RSTGEN_GENERAL_RST_OFF              (GENERAL_REG)
#define RSTGEN_ISO_EN_OFF(iso_idx)          (ISO_EN + 0x04 * iso_idx)
#define RSTGEN_MODULE_RST_OFF(module_idx)   (MODULE_RST + 0x4 * (module_idx))
#define SOC_RSTGEN_REG_MAP(reg_off)         ((reg_off) << 10)


#ifndef ASSEMBLY

__BEGIN_CDECLS

/**
 * @brief sdrv rstgen init
 *
 * @param[in] base sdrv rstgen address
 * @param[in] glb_rst_mask value for global reset enable register
 */
void sdrv_rstgen_lld_init(uint32_t base, uint32_t glb_rst_mask);

/**
 * @brief sdrv rstgen reset current domain
 *  RSTGEN_SAF: saf.sef_rst_trig reset safety and AP domain
 *  RSTGEN_RTC: rtc.self_rst_trig reset the RTC domain
 *  RSTGEN_SEC: ap.self_rst_trig reset the AP domain (sec & ap)
 *
 * @param[in] base sdrv rstgen address
 * @param[in] release release bit
 */
int32_t sdrv_rstgen_lld_sw_self_rst(uint32_t base, bool release);

/**
 * @brief sdrv rstgen reset other domain
 *  RSTGEN_SAF: saf.oth_rst_b reset the AP domain (sec & ap)
 *  RSTGEN_RTC and RSTGEN_SEC: oth reset has no effect
 *
 * @param[in] base sdrv rstgen address
 * @param[in] release release bit
 */
int32_t sdrv_rstgen_lld_sw_other_rst(uint32_t base, bool release);

/**
 * @brief sdrv rstgen core control
 * Reset core processor.
 *
 * static reset: SW controls STATIC_RST bit manually. SW write 0
 * to STATIC_RST to assert reset, or write 1 to STATIC_RST to release
 * the core from reset.
 *
 * NOTE: HW doesn't check bus status before resetting the core. SW must make
 * sure there's no active bus transaction on the core.
 *
 * @param[in] base sdrv rstgen address
 * @param[in] core_idx core index
 * @param[in] release release bit
 */
int32_t sdrv_rstgen_lld_core_control(uint32_t base, uint32_t core_idx, bool release);

/**
 * @brief sdrv rstgen core reset
 * Reset core processor.
 *
 * auto clear reset: SW write 1 to AUTO_CLR bit to reset the core.
 * RSTGEN assert and release the reset signal automatically.
 *
 * NOTE: HW doesn't check bus status before resetting the core. SW must make
 * sure there's no active bus transaction on the core.
 *
 * @param[in] base sdrv rstgen address
 * @param[in] core_idx core index
 */
int32_t sdrv_rstgen_lld_core_reset(uint32_t base, uint32_t core_idx);

/**
 * @brief sdrv rstgen module control
 * Reset IP module.
 *
 * SW write 0 to MODULE_RST_B_idx[1:1] to assert the reset or withe 1 to
 * MODULE_RST_B_idx[1:1] to release the reset.
 *
 * RSTGEN doesn't support auto-clear IP reset, so SW must assert the reset
 * signal first (release = 0), then release te reset signal (release = 1).
 * No delay is required, since this function polls reset signal.
 *
 * @param[in] base sdrv rstgen address
 * @param[in] module_idx module index
 * @param[in] release release bit
 */
int32_t sdrv_rstgen_lld_module_control(uint32_t base, uint32_t module_idx, bool release);

/**
 * @brief sdrv rstgen isolate control
 *
 * @param[in] base sdrv rstgen address
 * @param[in] iso_idx isolate index
 * @param[in] disable 0:isolate enable, 1:isolate disable
 */
int32_t sdrv_rstgen_lld_isolate_control(uint32_t base, uint32_t iso_idx, bool disable);

/**
 * @brief sdrv rstgen get core reset status
 *
 * @param[in] base sdrv rstgen address
 * @param[in] core_idx core index
 * @return 1, the core reset has be released.
 *         0, the core reset hasn't be released.
 */
int32_t sdrv_rstgen_lld_get_core_reset_status(uint32_t base, uint32_t core_idx);

/**
 * @brief sdrv rstgen get module reset status
 *
 * @param[in] base sdrv rstgen address
 * @param[in] module_idx module index
 * @return 1, the module reset has be released.
 *         0, the module reset hasn't be released.
 */
int32_t sdrv_rstgen_lld_get_module_reset_status(uint32_t base, uint32_t module_idx);

__END_CDECLS

#endif /* ASSEMBLY */

#endif /* DRV_LLD_SDRV_RSTGEN_H */
