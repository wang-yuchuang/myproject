/**
 * @file sdrv_reset_drv.c
 * @brief semidrive reset driver
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <types.h>
#include <bits.h>
#include <debug.h>
#include <reg.h>

#include "driver/kunlun/reset/sdrv_reset_drv.h"
#include "kunlun/reset/sdrv_rstgen.h"

static int32_t sdrv_reset_release_ctl(reset_ctl_t *rst_ctl, uint32_t id, bool release)
{
    int32_t ret = -1;
    sdrv_reset_ctl_t *sdrv_rst =
      (sdrv_reset_ctl_t *)rst_ctl->priv;

    switch (SDRV_RESET_TYPE(id)) {
        case SDRV_RESET_CORE:
            ret = sdrv_rstgen_lld_core_control(
                sdrv_rst->base, SDRV_RESET_INDEX(id), release);
        break;

        case SDRV_RESET_MODULE:
            ret = sdrv_rstgen_lld_module_control(
                sdrv_rst->base, SDRV_RESET_INDEX(id), release);
        break;

        case SDRV_RESET_ISOLATE:
            ret = sdrv_rstgen_lld_isolate_control(
                sdrv_rst->base, SDRV_RESET_INDEX(id), release);
        break;

        case SDRV_RESET_SELF:
            ret = sdrv_rstgen_lld_sw_self_rst(sdrv_rst->base, release);
        break;

        case SDRV_RESET_OTHER:
            ret = sdrv_rstgen_lld_sw_other_rst(sdrv_rst->base, release);
        break;

        default:
        break;
    }

    return ret;
}

static int32_t sdrv_reset_initialize(reset_ctl_t *rst_ctl)
{
    uint32_t glb_enable_cfg = 0x0u;
    sdrv_reset_ctl_t *sdrv_rst =
      (sdrv_reset_ctl_t *)rst_ctl->priv;

    glb_enable_cfg |= ((sdrv_rst->self_rst_en & 0x1u) << RSTGEN_GLB_RST_SELF_RST_EN);
    glb_enable_cfg |= ((sdrv_rst->sem_rst_en & 0x1u) << RSTGEN_GLB_RST_SEM_RST_EN);
    glb_enable_cfg |= ((sdrv_rst->dbg_rst_en & 0x1u) << RSTGEN_GLB_RST_DBG_RST_EN);
    glb_enable_cfg |= ((sdrv_rst->wdg1_rst_en & 0x1u) << RSTGEN_GLB_RST_WDG1_RST_EN);
    glb_enable_cfg |= ((sdrv_rst->wdg2_rst_en & 0x1u) << RSTGEN_GLB_RST_WDG2_RST_EN);
    glb_enable_cfg |= ((sdrv_rst->wdg3_rst_en & 0x1u) << RSTGEN_GLB_RST_WDG3_RST_EN);
    glb_enable_cfg |= ((sdrv_rst->wdg4_rst_en & 0x1u) << RSTGEN_GLB_RST_WDG4_RST_EN);
    glb_enable_cfg |= ((sdrv_rst->wdg5_rst_en & 0x1u) << RSTGEN_GLB_RST_WDG5_RST_EN);
    glb_enable_cfg |= ((sdrv_rst->wdg6_rst_en & 0x1u) << RSTGEN_GLB_RST_WDG6_RST_EN);
    glb_enable_cfg |= ((sdrv_rst->wdg7_rst_en & 0x1u) << RSTGEN_GLB_RST_WDG7_RST_EN);
    glb_enable_cfg |= ((sdrv_rst->wdg8_rst_en & 0x1u) << RSTGEN_GLB_RST_WDG8_RST_EN);

    sdrv_rstgen_lld_init(sdrv_rst->base, glb_enable_cfg);

    return 0;
}

static int32_t sdrv_reset_assert(reset_ctl_t *rst_ctl, uint32_t id)
{
    return sdrv_reset_release_ctl(rst_ctl, id, false);
}

static int32_t sdrv_reset_deassert(reset_ctl_t *rst_ctl, uint32_t id)
{
    return sdrv_reset_release_ctl(rst_ctl, id, true);
}

static int32_t sdrv_reset_reset(reset_ctl_t *rst_ctl, uint32_t id)
{
    int32_t ret = -1;
    sdrv_reset_ctl_t *sdrv_rst =
      (sdrv_reset_ctl_t *)rst_ctl->priv;

    switch (SDRV_RESET_TYPE(id)) {
        case SDRV_RESET_CORE:
            ret = sdrv_rstgen_lld_core_reset(sdrv_rst->base, SDRV_RESET_INDEX(id));
        break;

        case SDRV_RESET_MODULE:
            ret = sdrv_rstgen_lld_module_control(
                sdrv_rst->base, SDRV_RESET_INDEX(id), false);

            if (ret == 0) {
                ret = sdrv_rstgen_lld_module_control(
                    sdrv_rst->base, SDRV_RESET_INDEX(id), true);
            }
        break;

        default:
        break;
    }

    return ret;
}

static int32_t sdrv_reset_status(reset_ctl_t *rst_ctl, uint32_t id)
{
    int32_t ret = -1;
    sdrv_reset_ctl_t *sdrv_rst =
      (sdrv_reset_ctl_t *)rst_ctl->priv;

    switch (SDRV_RESET_TYPE(id)) {
        case SDRV_RESET_CORE:
            ret = !sdrv_rstgen_lld_get_core_reset_status(
                sdrv_rst->base, SDRV_RESET_INDEX(id));
        break;

        case SDRV_RESET_MODULE:
            ret = !sdrv_rstgen_lld_get_module_reset_status(
                sdrv_rst->base, SDRV_RESET_INDEX(id));
        break;

        default:
        break;
    }

    return ret;
}

static reset_ctl_ops_t ctl_ops = {
    .init     =  sdrv_reset_initialize,
    .assert   =  sdrv_reset_assert,
    .deassert =  sdrv_reset_deassert,
    .reset    =  sdrv_reset_reset,
    .status   =  sdrv_reset_status,
};

/**
 * @brief Initialize the  SemiDrive RESET controller.
 *
 * @param[in] rst_ctl Common reset device.
 * @param[in] sdrv_rst_ctl SemiDrive reset device.
 * @return 0 if OK, or a negative error code.
 */
int32_t sdrv_reset_init(reset_ctl_t *rst_ctl, sdrv_reset_ctl_t *sdrv_rst_ctl)
{
    rst_ctl->ops = &ctl_ops;
    rst_ctl->priv = sdrv_rst_ctl;
    return reset_ctl_init(rst_ctl);
}
