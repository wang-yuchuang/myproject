/**
 * @file sdrv_rstgen.c
 * @brief semidrive rstgen driver
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <types.h>

#include "reg.h"
#include "kunlun/reset/sdrv_rstgen.h"

/**
 * @brief sdrv rstgen init
 *
 * @param[in] base sdrv rstgen address
 * @param[in] glb_rst_mask value for global reset enable register
 */
void sdrv_rstgen_lld_init(uint32_t base, uint32_t glb_rst_mask)
{
    uint32_t reg_read = 0;
    uint32_t reg_write = 0;

    //clear all global register
    reg_write = 0x00000000;
    writel(reg_write, base);

    //unlock
    reg_read = readl(base);
    reg_write = reg_read & ~(0x1u << RSTGEN_GLB_RST_LOCK);
    writel(reg_write, base);

    reg_read = readl(base);
    reg_write = reg_read | glb_rst_mask;
    writel(reg_write, base);
}

/**
 * @brief sdrv rstgen reset current domain
 *  RSTGEN_SAF: saf.sef_rst_trig reset safety and AP domain
 *  RSTGEN_RTC: rtc.self_rst_trig reset the RTC domain
 *  RSTGEN_SEC: ap.self_rst_trig reset the AP domain (sec & ap)
 *
 * @param[in] base sdrv rstgen address
 * @param[in] release release bit
 */
int32_t sdrv_rstgen_lld_sw_self_rst(uint32_t base, bool release)
{
    uint32_t sw_self_rst = base + SOC_RSTGEN_REG_MAP(RSTGEN_SW_SELF_RST_OFF);

    /* Unlock self reset register. */
    RMWREG32(sw_self_rst, RSTGEN_SELF_RST_SW_GLB_RST_LOCK, 1, 0);

    /* Configure rst bit. */
    RMWREG32(sw_self_rst, RSTGEN_SELF_RST_SW_GLB_RST, 1, (uint32_t)release);

    return 0;
}

/**
 * @brief sdrv rstgen reset other domain
 *  RSTGEN_SAF: saf.oth_rst_b reset the AP domain (sec & ap)
 *  RSTGEN_RTC and RSTGEN_SEC: oth reset has no effect
 *
 * @param[in] base sdrv rstgen address
 * @param[in] release release bit
 */
int32_t sdrv_rstgen_lld_sw_other_rst(uint32_t base, bool release)
{
    uint32_t sw_oth_rst = base + SOC_RSTGEN_REG_MAP(RSTGEN_SW_OTH_RST_OFF);

    /* Unlock the register. */
    RMWREG32(sw_oth_rst, RSTGEN_OTH_RST_SW_GLB_RST_LOCK, 1, 0);

    /* Configure rst bit. */
    RMWREG32(sw_oth_rst, RSTGEN_OTH_RST_SW_GLB_RST, 1, (uint32_t)release);

    return 0;
}

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
int32_t sdrv_rstgen_lld_core_control(uint32_t base, uint32_t core_idx, bool release)
{
    uint32_t core_rst_en = base + SOC_RSTGEN_REG_MAP(
                              RSTGEN_CORE_RST_EN_OFF(core_idx));
    uint32_t core_sw_rst = base + SOC_RSTGEN_REG_MAP(
                              RSTGEN_CORE_SW_RST_OFF(core_idx));

    /* Check the reset lock bit */
    if (readl(core_rst_en) & (0x1u << RSTGEN_CORE_RST_RST_LOCK))
        return -1;

    /* Check core SW reset status */
    if (release && (readl(core_sw_rst) & (0x1u << RSTGEN_CORE_SW_RST_CORE_RST_STA)))
        return 0;

    if (!release && !(readl(core_sw_rst) & (0x1u << RSTGEN_CORE_SW_RST_CORE_RST_STA)))
        return 0;

    uint32_t sw_rst = release ? 1 : 0;

    /* Enable SW core reset. */
    RMWREG32(core_rst_en, RSTGEN_CORE_RST_SW_RST_EN, 1, 1);

    while (!(readl(core_rst_en) & (0x1u << RSTGEN_CORE_RST_SW_RST_EN_STA)));

    /* STATIC_RST_B. */
    RMWREG32(core_sw_rst, RSTGEN_CORE_SW_RST_STATIC_RST, 1, sw_rst);

    /* Wait reset done. */
    while (sw_rst != ((readl(core_sw_rst) >> RSTGEN_CORE_SW_RST_CORE_RST_STA) & 0x1u));

    return 0;
}

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
int32_t sdrv_rstgen_lld_core_reset(uint32_t base, uint32_t core_idx)
{
    uint32_t core_rst_en = base + SOC_RSTGEN_REG_MAP(
                              RSTGEN_CORE_RST_EN_OFF(core_idx));
    uint32_t core_sw_rst = base + SOC_RSTGEN_REG_MAP(
                              RSTGEN_CORE_SW_RST_OFF(core_idx));

    /* Check the reset lock bit */
    if (readl(core_rst_en) & (0x1u << RSTGEN_CORE_RST_RST_LOCK))
        return -1;

    /* Release STATIC_RST_B. */
    RMWREG32(core_sw_rst, RSTGEN_CORE_SW_RST_STATIC_RST, 1, 1);

    while (!(readl(core_sw_rst) &
            (0x1u << RSTGEN_CORE_SW_RST_STATIC_RST_B_STA)));

    /* Enable SW core reset. */
    RMWREG32(core_rst_en, RSTGEN_CORE_RST_SW_RST_EN, 1, 1);

    while (!(readl(core_rst_en) &
            (0x1u << RSTGEN_CORE_RST_SW_RST_EN_STA)));

    /* Trigger auto-clear reset. */
    RMWREG32(core_sw_rst, RSTGEN_CORE_SW_RST_AUTO_CLR, 1, 1);

    /* Polling reset status */
    while (!(readl(core_sw_rst) &
            (0x1u << RSTGEN_CORE_SW_RST_CORE_RST_STA)));

    return 0;
}

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
int32_t sdrv_rstgen_lld_module_control(uint32_t base, uint32_t module_idx, bool release)
{
    uint32_t module_rst = base + SOC_RSTGEN_REG_MAP(
                             RSTGEN_MODULE_RST_OFF(module_idx));

    if (readl(module_rst) & (0x1u << RSTGEN_MODULE_RST_LOCK))
        return -1;

    if (release && (readl(module_rst) & (0x1u << RSTGEN_MODULE_RST_STA)))
        return 0;

    if (!release && !(readl(module_rst) & (0x1u << RSTGEN_MODULE_RST_STA)))
        return 0;

    /* Enable module reset. */
    RMWREG32(module_rst, RSTGEN_MODULE_RST_EN, 1, 1);

    while (!(readl(module_rst) & (0x1u << RSTGEN_MODULE_RST_EN)));

    /* Write RST_N bit. 0 = reset, 1 = release. */
    uint32_t rst_n = release ? 1 : 0;

    RMWREG32(module_rst, RSTGEN_MODULE_RST_N, 1, rst_n);

    /* Wait module reset done. */
    while (rst_n != ((readl(module_rst) >> RSTGEN_MODULE_RST_STA) & 0x1u));

    return 0;
}

/**
 * @brief sdrv rstgen isolate control
 *
 * @param[in] base sdrv rstgen address
 * @param[in] iso_idx isolate index
 * @param[in] disable 0:isolate enable, 1:isolate disable
 */
int32_t sdrv_rstgen_lld_isolate_control(uint32_t base, uint32_t iso_idx, bool disable)
{
    uint32_t iso_addr;

    iso_addr = base + SOC_RSTGEN_REG_MAP(ISO_EN + 0x04 * iso_idx);

    if (disable)
        writel((0x1u << RSTGEN_ISO_EN_B), iso_addr);
    else
        writel(~(0x1u << RSTGEN_ISO_EN_B), iso_addr);

    return 0;
}

/**
 * @brief sdrv rstgen get core reset status
 *
 * @param[in] base sdrv rstgen address
 * @param[in] core_idx core index
 * @return 1, the core reset has be released.
 *         0, the core reset hasn't be released.
 */
int32_t sdrv_rstgen_lld_get_core_reset_status(uint32_t base, uint32_t core_idx)
{
    uint32_t core_sw_rst = base + SOC_RSTGEN_REG_MAP(
                RSTGEN_CORE_SW_RST_OFF(core_idx));

    return !!(readl(core_sw_rst) & (0x1u << RSTGEN_CORE_SW_RST_CORE_RST_STA));
}

/**
 * @brief sdrv rstgen get module reset status
 *
 * @param[in] base sdrv rstgen address
 * @param[in] module_idx module index
 * @return 1, the module reset has be released.
 *         0, the module reset hasn't be released.
 */
int32_t sdrv_rstgen_lld_get_module_reset_status(uint32_t base, uint32_t module_idx)
{
    uint32_t module_rst = base + SOC_RSTGEN_REG_MAP(
                             RSTGEN_MODULE_RST_OFF(module_idx));

    /* RSTGEN_MODULE_RST_STA, 0 = reset, 1 = release. */
    return !!(readl(module_rst) & (0x1u << RSTGEN_MODULE_RST_STA));
}