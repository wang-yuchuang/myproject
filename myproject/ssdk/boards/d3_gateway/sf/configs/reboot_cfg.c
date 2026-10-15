
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_TAISHAN_REBOOT

#include <types.h>
#include <param.h>
#include <scr_hw.h>
#include <rstgen_cfg.h>
#include <scr_cfg.h>
#include <regs_base.h>
#include <lib/disk/disk.h>
#include <driver/scr.h>
#include <reboot.h>
#include "reboot_cfg.h"

#if CONFIG_SP_SPLIT_MODE
static scr_signal_t sp0_scr_signal[] = {
    /* sp_split_scr */
    SCR_SF_VIC2_IS_LOCKSTEP_MODE_P,
    SCR_SF_SCR_CR5_SP_LS_DISABLE,
    SCR_SF_SCR_CR5_SP_SLCLAMP_DISABLE,
    /* sp0_remap_scr */
    SCR_SF_REMAP_CR5_SP_AR0_ADDR_OFFSET_19_0,
    SCR_SF_REMAP_CR5_SP_AR0_REMAP_EN,
    SCR_SF_REMAP_CR5_SP_AW0_ADDR_OFFSET_19_0,
    SCR_SF_REMAP_CR5_SP_AW0_REMAP_EN,
};

static uint32_t sp0_scr_value[] = {
    /* sp_split_scr */
    1,
    1,
    1,
    /* sp0_remap_scr */
    CONFIG_SP0_MEM_BASE >> 12,
    1,
    CONFIG_SP0_MEM_BASE >> 12,
    1,
};

static scr_signal_t sp1_scr_signal[] = {
    /* sp_split_scr */
    SCR_SF_VIC2_IS_LOCKSTEP_MODE_P,
    SCR_SF_SCR_CR5_SP_LS_DISABLE,
    SCR_SF_SCR_CR5_SP_SLCLAMP_DISABLE,
    /* sp1_remap_scr */
    SCR_SF_REMAP_CR5_SP_AR1_ADDR_OFFSET_19_0,
    SCR_SF_REMAP_CR5_SP_AR1_REMAP_EN,
    SCR_SF_REMAP_CR5_SP_AW1_ADDR_OFFSET_19_0,
    SCR_SF_REMAP_CR5_SP_AW1_REMAP_EN,
};

static uint32_t sp1_scr_value[] = {
    /* sp_split_scr */
    1,
    1,
    1,
    /* sp1_remap_scr */
    CONFIG_SP1_MEM_BASE >> 12,
    1,
    CONFIG_SP1_MEM_BASE >> 12,
    1,
};
static struct reboot_cfg reboot_sp0 = {
    .id = SP0,
    .target_addr = CONFIG_SP0_MEM_BASE,
    .target_len = CONFIG_SP0_MEM_LEN,
#if CONFIG_REBOOT_DISK_NORFLASH
    .disk_name = DISK_NOR_FLASH_NAME(0),
#elif CONFIG_REBOOT_DISK_MMC0
    .disk_name = DISK_MMC_NAME(0),
#else
    .disk_name = DISK_MMC_NAME(1),
#endif
    .pt_name = CONFIG_SP0_PT_NAME,
    .scr_entry_signal = {
        &g_scr_sf,
        sp0_scr_signal,
        sp0_scr_value,
        ARRAY_SIZE(sp0_scr_signal)
    },
    .rst_cpu = &rstsig_cr5_sp0
};

static struct reboot_cfg reboot_sp1 = {
    .id = SP1,
    .target_addr = CONFIG_SP1_MEM_BASE,
    .target_len = CONFIG_SP1_MEM_LEN,
#if CONFIG_REBOOT_DISK_NORFLASH
    .disk_name = DISK_NOR_FLASH_NAME(0),
#elif CONFIG_REBOOT_DISK_MMC0
    .disk_name = DISK_MMC_NAME(0),
#else
    .disk_name = DISK_MMC_NAME(1),
#endif
    .pt_name = CONFIG_SP1_PT_NAME,
    .scr_entry_signal = {
        &g_scr_sf,
        sp1_scr_signal,
        sp1_scr_value,
        ARRAY_SIZE(sp1_scr_signal)
    },
    .rst_cpu = &rstsig_cr5_sp1
};

#elif CONFIG_SP_LOCKSTEP_MODE
static scr_signal_t sp_scr_signal[] = {
    /* sp_split_scr */
    SCR_SF_VIC2_IS_LOCKSTEP_MODE_P,
    SCR_SF_SCR_CR5_SP_LS_DISABLE,
    SCR_SF_SCR_CR5_SP_SLCLAMP_DISABLE,
    /* sp_remap_scr */
    SCR_SF_REMAP_CR5_SP_AR0_ADDR_OFFSET_19_0,
    SCR_SF_REMAP_CR5_SP_AR0_REMAP_EN,
    SCR_SF_REMAP_CR5_SP_AW0_ADDR_OFFSET_19_0,
    SCR_SF_REMAP_CR5_SP_AW0_REMAP_EN,
};

static uint32_t sp_scr_value[] = {
    /* sp_split_scr */
    0,
    0,
    0,
    /* sp_remap_scr */
    CONFIG_SP_MEM_BASE >> 12,
    1,
    CONFIG_SP_MEM_BASE >> 12,
    1,
};

static struct reboot_cfg reboot_sp = {
    .id = SP,
    .target_addr = CONFIG_SP_MEM_BASE,
    .target_len = CONFIG_SP_MEM_LEN,
#if CONFIG_REBOOT_DISK_NORFLASH
    .disk_name = DISK_NOR_FLASH_NAME(0),
#elif CONFIG_REBOOT_DISK_MMC0
    .disk_name = DISK_MMC_NAME(0),
#else
    .disk_name = DISK_MMC_NAME(1),
#endif
    .pt_name = CONFIG_SP_PT_NAME,
    .scr_entry_signal = {
        &g_scr_sf,
        sp_scr_signal,
        sp_scr_value,
        ARRAY_SIZE(sp_scr_signal)
    },
    .rst_cpu = &rstsig_cr5_sp0
};
#endif

#if CONFIG_SX_SPLIT_MODE
static scr_signal_t sx0_scr_signal[] = {
    /* sx_split_scr */
    SCR_SF_VIC3_IS_LOCKSTEP_MODE_P,
    SCR_SF_SCR_CR5_SX_LS_DISABLE,
    SCR_SF_SCR_CR5_SX_SLCLAMP_DISABLE,
    /* sx0_remap_scr */
    SCR_SF_REMAP_CR5_SX_AR0_ADDR_OFFSET_19_0,
    SCR_SF_REMAP_CR5_SX_AR0_REMAP_EN,
    SCR_SF_REMAP_CR5_SX_AW0_ADDR_OFFSET_19_0,
    SCR_SF_REMAP_CR5_SX_AW0_REMAP_EN,
};

static uint32_t sx0_scr_value[] = {
    /* sx_split_scr */
    1,
    1,
    1,
    /* sx0_remap_scr */
    CONFIG_SX0_MEM_BASE >> 12,
    1,
    CONFIG_SX0_MEM_BASE >> 12,
    1,
};

static scr_signal_t sx1_scr_signal[] = {
    /* sx_split_scr */
    SCR_SF_VIC3_IS_LOCKSTEP_MODE_P,
    SCR_SF_SCR_CR5_SX_LS_DISABLE,
    SCR_SF_SCR_CR5_SX_SLCLAMP_DISABLE,
    /* sx1_remap_scr */
    SCR_SF_REMAP_CR5_SX_AR1_ADDR_OFFSET_19_0,
    SCR_SF_REMAP_CR5_SX_AR1_REMAP_EN,
    SCR_SF_REMAP_CR5_SX_AW1_ADDR_OFFSET_19_0,
    SCR_SF_REMAP_CR5_SX_AW1_REMAP_EN,
};

static uint32_t sx1_scr_value[] = {
    /* sp_split_scr */
    1,
    1,
    1,
    /* sp1_remap_scr */
    CONFIG_SX1_MEM_BASE >> 12,
    1,
    CONFIG_SX1_MEM_BASE >> 12,
    1,
};
static struct reboot_cfg reboot_sx0 = {
    .id = SX0,
    .target_addr = CONFIG_SX0_MEM_BASE,
    .target_len = CONFIG_SX0_MEM_LEN,
#if CONFIG_REBOOT_DISK_NORFLASH
    .disk_name = DISK_NOR_FLASH_NAME(0),
#elif CONFIG_REBOOT_DISK_MMC0
    .disk_name = DISK_MMC_NAME(0),
#else
    .disk_name = DISK_MMC_NAME(1),
#endif
    .pt_name = CONFIG_SX0_PT_NAME,
    .scr_entry_signal = {
        &g_scr_sf,
        sx0_scr_signal,
        sx0_scr_value,
        ARRAY_SIZE(sx0_scr_signal)
    },
    .rst_cpu = &rstsig_cr5_sx0
};

static struct reboot_cfg reboot_sx1 = {
    .id = SX1,
    .target_addr = CONFIG_SX1_MEM_BASE,
    .target_len = CONFIG_SX1_MEM_LEN,
#if CONFIG_REBOOT_DISK_NORFLASH
    .disk_name = DISK_NOR_FLASH_NAME(0),
#elif CONFIG_REBOOT_DISK_MMC0
    .disk_name = DISK_MMC_NAME(0),
#else
    .disk_name = DISK_MMC_NAME(1),
#endif
    .pt_name = CONFIG_SX1_PT_NAME,
    .scr_entry_signal = {
        &g_scr_sf,
        sx1_scr_signal,
        sx1_scr_value,
        ARRAY_SIZE(sx1_scr_signal)
    },
    .rst_cpu = &rstsig_cr5_sx1
};
#elif CONFIG_SX_LOCKSTEP_MODE
static scr_signal_t sx_scr_signal[] = {
    /* sx_split_scr */
    SCR_SF_VIC3_IS_LOCKSTEP_MODE_P,
    SCR_SF_SCR_CR5_SX_LS_DISABLE,
    SCR_SF_SCR_CR5_SX_SLCLAMP_DISABLE,
    /* sx_remap_scr */
    SCR_SF_REMAP_CR5_SX_AR0_ADDR_OFFSET_19_0,
    SCR_SF_REMAP_CR5_SX_AR0_REMAP_EN,
    SCR_SF_REMAP_CR5_SX_AW0_ADDR_OFFSET_19_0,
    SCR_SF_REMAP_CR5_SX_AW0_REMAP_EN,
};

static uint32_t sx_scr_value[] = {
    /* sx_split_scr */
    0,
    0,
    0,
    /* sx_remap_scr */
    CONFIG_SX_MEM_BASE >> 12,
    1,
    CONFIG_SX_MEM_BASE >> 12,
    1,
};

static struct reboot_cfg reboot_sx = {
    .id = SX,
    .target_addr = CONFIG_SX_MEM_BASE,
    .target_len = CONFIG_SX_MEM_LEN,
#if CONFIG_REBOOT_DISK_NORFLASH
    .disk_name = DISK_NOR_FLASH_NAME(0),
#elif CONFIG_REBOOT_DISK_MMC0
    .disk_name = DISK_MMC_NAME(0),
#else
    .disk_name = DISK_MMC_NAME(1),
#endif
    .pt_name = CONFIG_SX_PT_NAME,
    .scr_entry_signal = {
        &g_scr_sf,
        sx_scr_signal,
        sx_scr_value,
        ARRAY_SIZE(sx_scr_signal)
    },
    .rst_cpu = &rstsig_cr5_sx0
};
#endif

static struct reboot_cfg *board_reboot_array[] = {
#if CONFIG_SP_SPLIT_MODE
    &reboot_sp0,
    &reboot_sp1,
#elif CONFIG_SP_LOCKSTEP_MODE
    &reboot_sp,
#endif
#if CONFIG_SX_SPLIT_MODE
    &reboot_sx0,
    &reboot_sx1,
#elif CONFIG_SX_LOCKSTEP_MODE
    &reboot_sx,
#endif
};

void board_reboot_init(void)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(board_reboot_array); i++) {
        reboot_ctrl(board_reboot_array[i]);
    }
}
#endif //CONFIG_TAISHAN_REBOOT
