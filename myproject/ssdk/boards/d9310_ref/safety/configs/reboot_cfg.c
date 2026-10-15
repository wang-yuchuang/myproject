
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_REBOOT

#include <types.h>
#include <param.h>
#include <string.h>
#include <arch/cache.h>
#include <reboot.h>
#include <regs_base.h>
#include <scr_hw.h>
#include <rstgen_cfg.h>
#include <scr_cfg.h>
#include <mbox_cfg.h>
#include <bits.h>
#include <reg.h>
#include <board.h>
#include <mem_image.h>

#include <CLI.h>
#include <clock_cfg.h>
#include <driver/clk.h>
#include "image_cfg.h"
#include "reboot_cfg.h"
#include "mbox_cfg.h"
#include "driver/kunlun/fuse/fuse.h"
#include <lib/sd_boot_pin/sd_boot_pin.h>

#define IMG_SEEK_BASE   DIL_IMAGES_MEMBASE
#define IMG_SEEK_SIZE   DIL_IMAGES_MEMSIZE

#define CPU2_ENTRY      AP2_PRELOADER_MEMBASE
#define CPU2_MEMBASE    (AP2_PRELOADER_MEMBASE - 0x10000000)
#define CONFIG_D9_SECURE_MEM_BASE IRAM2_BASE
#define SECURE_ENTRY    IRAM2_BASE

#define BPT_SIZE           0x800
#define BPT_TAG            0x42505401
#define IIB_OFFSET         0x20
#define IMG_SZ_IIB_OFFSET  0x28
#define IMG_LDA_IIB_OFFSET 0x2C
#define IMG_EP_IIB_OFFSET  0x34
#define GET_BPT_IMG_SZ(h)  (*((uint32_t *)(addr_t)((h) + IIB_OFFSET + IMG_SZ_IIB_OFFSET)))
#define GET_BPT_IMG_LDA(h) (*((uint32_t *)(addr_t)((h) + IIB_OFFSET + IMG_LDA_IIB_OFFSET)))
#define GET_BPT_IMG_EP(h)  (*((uint32_t *)(addr_t)((h) + IIB_OFFSET + IMG_EP_IIB_OFFSET)))
#define HAS_BPT_HEADER(h)  (*((uint32_t*)(addr_t)h) == BPT_TAG)

typedef struct tag_reboot_rst {
    int32_t (*rst_fun)(struct reset_sig *rst_sig);
    struct reset_sig* rst_sig;
} reboot_rst_t;

typedef struct tag_reboot_clk {
    int32_t (*clk_fun)(clk_node_t* clk);
    clk_node_t* clk;
}reboot_clk_t;

/* reboot sec config structures */
static scr_signal_t sec_scr_signal[] = {
    SCR_SEC__remap_cr5_sec_ar_addr_offset_19_0,
    SCR_SEC__remap_cr5_sec_ar_remap_ovrd_en
};

static uint32_t sec_scr_value[] = {
    SECURE_ENTRY >> 12,
    1
};

static struct reboot_cfg reboot_sec = {
    .id = CPU_ID_SEC,
    .seeker_base = IMG_SEEK_BASE,
    .seeker_size = IMG_SEEK_SIZE,
    .pt_name = "ssystem",
    .rst_cpu = &rstsig_cr5_sec,
    .scr_entry_signal = {
        .scr_dev = &g_scr_sec,
        .scr_sig = sec_scr_signal,
        .scr_value = sec_scr_value,
        .num = ARRAY_SIZE(sec_scr_signal),
    },
    .mbox = NULL,
};

/** reboot ap2 configs */
static reboot_rst_t s_ap2_pre_rst_arry[] = {
    {reset_ctl_assert,   &rstsig_cpu2_ss},
    {reset_ctl_assert,   &rstsig_cpu2},

    {reset_ctl_reset,    &rstsig_gic5},

    {reset_ctl_reset,    &rstsig_cpu2_ss},
    {NULL, NULL},
};

static reboot_clk_t s_ap2_clk_arry[] = {
    {clk_prepare,        &g_ckgen_core_cpu2_pre},

    {clk_prepare,        &g_ckgen_uuu_cpu2_div_m_pre},
    {clk_prepare,        &g_ckgen_uuu_cpu2_div_n_pre},
    {clk_prepare,        &g_ckgen_uuu_cpu2_div_p_pre},
    {clk_prepare,        &g_ckgen_uuu_cpu2_div_q_pre},

    {clk_enable,         &g_ckgen_gating_pll_cpu2},

    {clk_prepare,        &g_pll_cpu2_vco},
    {clk_prepare,        &g_pll_cpu2_root},
    {clk_prepare,        &g_pll_cpu2_div_a},
    {clk_prepare,        &g_pll_cpu2_div_b},

    {clk_prepare,        &g_ckgen_uuu_cpu2_div_m},
    {clk_prepare,        &g_ckgen_uuu_cpu2_div_n},
    {clk_prepare,        &g_ckgen_uuu_cpu2_div_p},
    {clk_prepare,        &g_ckgen_uuu_cpu2_div_q},

    {clk_enable,         &g_ckgen_gating_cpu2},
    {NULL, NULL},
};

static scr_signal_t ap2_scr_signal[] = {
    SCR_SEC__cpu2_rvbaraddr0_29_2,
    SCR_SEC__cpu2_rvbaraddr0_39_30,
};

static uint32_t ap2_scr_value[] = {
    // 0x1800000,
    // 0x3
    (CPU2_ENTRY & BIT_MASK(30)) >> 2,
    (CPU2_ENTRY >> 30)
};

static struct reboot_cfg reboot_ap2 = {
    .id = CPU_ID_CPU2,
    .seeker_base = IMG_SEEK_BASE,
    .seeker_size = IMG_SEEK_SIZE,
    .pt_name = "preloader",
    .rst_cpu = &rstsig_cpu2,
    .scr_entry_signal = {
        .scr_dev = &g_scr_sec,
        .scr_sig = ap2_scr_signal,
        .scr_value = ap2_scr_value,
        .num = ARRAY_SIZE(ap2_scr_signal),
    },
    .target_addr = CPU2_MEMBASE,
    .mbox = NULL,
};

static int32_t boot_secure()
{
    uint32_t ret;
    size_t seeker_size = reboot_sec.seeker_size;
    addr_t seeker_base = reboot_sec.seeker_base;
    mem_image_entry_t info = {0};

    mem_image_init(seeker_base, seeker_size);
    ret = mem_image_seek(seeker_base, reboot_sec.pt_name, &info);

    if (ret != 0) {
        ssdk_printf(SSDK_EMERG, "not found invalid image for %s\n", reboot_sec.pt_name);
        return -1;
    }
    // copy image
    memcpy((void *)(addr_t)CONFIG_D9_SECURE_MEM_BASE,
           (void *)(addr_t)(info.base), info.sz);

    arch_clean_cache_range((addr_t)CONFIG_D9_SECURE_MEM_BASE, info.sz);

    reboot_sec.target_addr = CONFIG_D9_SECURE_MEM_BASE;

    // update scr value
    reboot_sec.scr_entry_signal.scr_value[0] = CONFIG_D9_SECURE_MEM_BASE >> 12;
    reboot_sec.scr_entry_signal.scr_value[1] = 1;
    if (boot_get_pin() == BOOT_PIN_0){
        reboot_sec.mbox = g_mbox_dev;
        reboot_ctrl(&reboot_sec);
    } else {
        // reboot secure
        reboot_ctrl(&reboot_sec);
    }

    return 0;
}

int32_t boot_ap1()
{
    printf("d9l_ref board ap1 boot is not supported!\n");
    return 0;
}

int32_t boot_ap2()
{
    int32_t cnt = 0;

    // module reset
    while (s_ap2_pre_rst_arry[cnt].rst_fun != NULL) {
        s_ap2_pre_rst_arry[cnt].rst_fun(s_ap2_pre_rst_arry[cnt].rst_sig);
        cnt++;
    }

    // clock prepare & enable
    cnt = 0;
    while (s_ap2_clk_arry[cnt].clk_fun != NULL) {
        s_ap2_clk_arry[cnt].clk_fun(s_ap2_clk_arry[cnt].clk);
        cnt++;
    }

    reboot_ctrl(&reboot_ap2);
    return 0;
}

void board_reboot_init(void)
{
    boot_secure();
    boot_ap2();
}

#endif
