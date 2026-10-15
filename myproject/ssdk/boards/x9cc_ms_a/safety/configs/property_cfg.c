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

struct sys_property_value ss_properties[] = {
    { .id = DMP_ID_DDR_STATUS,          .flags = SYS_PROP_F_RO, .owner = DP_CR5_SAF },
    { .id = DMP_ID_PORT_STATUS,         .flags = SYS_PROP_F_RO, .owner = DP_CR5_SAF },
    { .id = DMP_ID_PLL_CLK_STATUS,      .flags = SYS_PROP_F_RO, .owner = DP_CR5_SAF },
    { .id = DMP_ID_HANDOVER_STATUS,     .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF },
    { .id = DMP_ID_DC_STATUS,           .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF },
    { .id = DMP_ID_VDSP_STATUS,         .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF },
    { .id = DMP_ID_VPU_STATUS,          .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF },
    { .id = DMP_ID_I2C_STATUS,          .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF },
    { .id = DMP_ID_REBOOT_STATUS,       .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF },
    { .id = DMP_ID_AVM_STATUS,          .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF },
    { .id = DMP_ID_BA_STATUS,           .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF },
    { .id = DMP_ID_DMS_STATUS,          .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF },
    { .id = DMP_ID_CLUSTER_STATUS,      .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF },
    { .id = DMP_ID_CP_STATUS,           .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF },
    { .id = DMP_ID_RVM_STATUS,          .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF },
    { .id = DMP_ID_UPDATE_STATUS,       .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF },
    { .id = DMP_ID_ANDROID_BOOT_STATUS, .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF },
    { .id = DMP_ID_CONTROLPANEL_STATUS, .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF },
    { .id = DMP_ID_CVBS_STATUS,         .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF },
    { .id = DMP_ID_RECOVERY_STATUS,     .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF },
    { .id = DMP_ID_TOUCH_STATUS,        .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF },
    { .id = DMP_ID_AVM_CAST_STATUS,     .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF },
    { .id = DMP_ID_OTA_STATUS,          .flags = SYS_PROP_F_WR, .owner = DP_CR5_SAF },
};

int board_property_get_this_proc()
{
    return DP_CR5_SAF;
}

int board_property_get_array_size()
{
    return (int)(ARRAY_SIZE(ss_properties));
}
#endif /* CONFIG_PROPERTY */
/* EOF */
