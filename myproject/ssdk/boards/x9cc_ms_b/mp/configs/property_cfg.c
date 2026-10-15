/*
 * property_cfg.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifdef CONFIG_PROPERTY
#include <debug.h>
#include <types.h>
#include <param.h>
#include "board.h"
#include "property_cfg.h"

#define define_prop_name(n) \
char n##_name[] = "cdm.rw."#n".status";
#define prop_name(n) n##_name

#if !SUPPORT_VDM
define_prop_name(ddr)
define_prop_name(port)
define_prop_name(pll_clk)
define_prop_name(handover)
define_prop_name(dc)
define_prop_name(vdsp)
define_prop_name(vpu)
define_prop_name(i2c)
define_prop_name(reboot)
define_prop_name(avm)
define_prop_name(ba)
define_prop_name(dms)
define_prop_name(cluster)
define_prop_name(cp)
define_prop_name(rvm)
define_prop_name(update)
define_prop_name(android_boot)
define_prop_name(controlpanel)
define_prop_name(cvbs)
define_prop_name(recovery)
define_prop_name(touch)
define_prop_name(avm_cast)
define_prop_name(ota)
#endif


struct sys_property_value ss_properties[] = {
    {.id = DMP_ID_DDR_STATUS,          .name = prop_name(ddr),          .flags = SYS_PROP_F_RO, .owner = DP_CR5_MPC},
    {.id = DMP_ID_PORT_STATUS,         .name = prop_name(port),         .flags = SYS_PROP_F_RO, .owner = DP_CR5_MPC},
    {.id = DMP_ID_PLL_CLK_STATUS,      .name = prop_name(pll_clk),      .flags = SYS_PROP_F_RO, .owner = DP_CR5_MPC},
    {.id = DMP_ID_HANDOVER_STATUS,     .name = prop_name(handover),     .flags = SYS_PROP_F_WR, .owner = DP_CR5_MPC},
    {.id = DMP_ID_DC_STATUS,           .name = prop_name(dc),           .flags = SYS_PROP_F_WR, .owner = DP_CR5_MPC},
    {.id = DMP_ID_VDSP_STATUS,         .name = prop_name(vdsp),         .flags = SYS_PROP_F_WR, .owner = DP_CR5_MPC},
    {.id = DMP_ID_VPU_STATUS,          .name = prop_name(vpu),          .flags = SYS_PROP_F_WR, .owner = DP_CR5_MPC},
    {.id = DMP_ID_I2C_STATUS,          .name = prop_name(i2c),          .flags = SYS_PROP_F_WR, .owner = DP_CR5_MPC},
    {.id = DMP_ID_REBOOT_STATUS,       .name = prop_name(reboot),       .flags = SYS_PROP_F_WR, .owner = DP_CR5_MPC},
    {.id = DMP_ID_AVM_STATUS,          .name = prop_name(avm),          .flags = SYS_PROP_F_WR, .owner = DP_CR5_MPC},
    {.id = DMP_ID_BA_STATUS,           .name = prop_name(ba),           .flags = SYS_PROP_F_WR, .owner = DP_CR5_MPC},
    {.id = DMP_ID_DMS_STATUS,          .name = prop_name(dms),          .flags = SYS_PROP_F_WR, .owner = DP_CR5_MPC},
    {.id = DMP_ID_CLUSTER_STATUS,      .name = prop_name(cluster),      .flags = SYS_PROP_F_WR, .owner = DP_CR5_MPC},
    {.id = DMP_ID_CP_STATUS,           .name = prop_name(cp),           .flags = SYS_PROP_F_WR, .owner = DP_CR5_MPC},
    {.id = DMP_ID_RVM_STATUS,          .name = prop_name(rvm),          .flags = SYS_PROP_F_WR, .owner = DP_CR5_MPC},
    {.id = DMP_ID_UPDATE_STATUS,       .name = prop_name(update),       .flags = SYS_PROP_F_WR, .owner = DP_CR5_MPC},
    {.id = DMP_ID_ANDROID_BOOT_STATUS, .name = prop_name(android_boot), .flags = SYS_PROP_F_WR, .owner = DP_CR5_MPC},
    {.id = DMP_ID_CONTROLPANEL_STATUS, .name = prop_name(controlpanel), .flags = SYS_PROP_F_WR, .owner = DP_CR5_MPC},
    {.id = DMP_ID_CVBS_STATUS,         .name = prop_name(cvbs),         .flags = SYS_PROP_F_WR, .owner = DP_CR5_MPC},
    {.id = DMP_ID_RECOVERY_STATUS,     .name = prop_name(recovery),     .flags = SYS_PROP_F_WR, .owner = DP_CR5_MPC},
    {.id = DMP_ID_TOUCH_STATUS,        .name = prop_name(touch),        .flags = SYS_PROP_F_WR, .owner = DP_CR5_MPC},
    {.id = DMP_ID_AVM_CAST_STATUS,     .name = prop_name(avm_cast),     .flags = SYS_PROP_F_WR, .owner = DP_CR5_MPC},
    {.id = DMP_ID_OTA_STATUS,          .name = prop_name(ota),          .flags = SYS_PROP_F_WR, .owner = DP_CR5_MPC},
};

int board_property_get_this_proc()
{
    return DP_CR5_MPC;
}

int board_property_get_array_size()
{
    return (int)(ARRAY_SIZE(ss_properties));
}
#endif /* CONFIG_PROPERTY */
/* EOF */
