
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

#define CPU1_ENTRY      AP1_PRELOADER_MEMBASE
#define CPU1_MEMBASE    (AP1_PRELOADER_MEMBASE - 0x10000000)

#define CPU2_ENTRY      AP2_PRELOADER_MEMBASE
#define CPU2_MEMBASE    (AP2_PRELOADER_MEMBASE - 0x10000000)

#define CONFIG_D9_SECURE_MEM_BASE IRAM2_BASE
#define SECURE_ENTRY    IRAM2_BASE
#define MP_ENTRY        MP_MEMBASE

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

/* reboot mp config structures */
static scr_signal_t mp_scr_signal[] = {
    SCR_SEC__remap_cr5_mp_ar_addr_offset_19_0,
    SCR_SEC__remap_cr5_mp_ar_remap_en
};

static uint32_t mp_scr_value[] = {
    MP_ENTRY >> 12,
    1
};

static struct reboot_cfg reboot_mp = {
    .id = CPU_ID_MP,
    .seeker_base = IMG_SEEK_BASE,
    .seeker_size = IMG_SEEK_SIZE,
    .pt_name = "mp_os",
    .rst_cpu = &rstsig_cr5_mp,
    .scr_entry_signal = {
        .scr_dev = &g_scr_sec,
        .scr_sig = mp_scr_signal,
        .scr_value = mp_scr_value,
        .num = ARRAY_SIZE(mp_scr_signal),
    },
    .target_addr = MP_ENTRY,
    .mbox = NULL,
};

/** reboot ap1 configs */
static reboot_rst_t s_ap1_pre_rst_arry[] = {
    {reset_ctl_assert,  &rstsig_cpu1_ss},
    {reset_ctl_assert,  &rstsig_cpu1_core0_warm},
    {reset_ctl_assert,  &rstsig_cpu1_core1_warm},
    {reset_ctl_assert,  &rstsig_cpu1_core2_warm},
    {reset_ctl_assert,  &rstsig_cpu1_core3_warm},
    {reset_ctl_assert,  &rstsig_cpu1_core4_warm},
    {reset_ctl_assert,  &rstsig_cpu1_core5_warm},
    {reset_ctl_assert,  &rstsig_cpu1_all},

    {reset_ctl_reset,   &rstsig_gic4},

    {reset_ctl_assert,   &rstsig_pciephy},
    {reset_ctl_assert,   &rstsig_pcie1},
    {reset_ctl_assert,   &rstsig_pcie2},
    {reset_ctl_deassert, &rstsig_pciephy},
    {reset_ctl_deassert, &rstsig_pcie1},
    {reset_ctl_deassert, &rstsig_pcie2},

    {reset_ctl_deassert, &rstsig_isolate_cpu1},
    {reset_ctl_reset,    &rstsig_cpu1_ss},
    {NULL,NULL}
};

static reboot_rst_t s_ap1_post_rst_arry[] = {
    {reset_ctl_reset,   &rstsig_cpu1_core0_warm},
    {reset_ctl_reset,   &rstsig_cpu1_scu_warm},
    {NULL, NULL}
};

static reboot_clk_t s_ap1_clk_arry[] = {
    {clk_enable,         &g_ckgen_gating_pll_cpu1},
    {clk_enable,         &g_ckgen_gating_gic4},
    {clk_prepare,        &g_ckgen_core_cpu1a_pre},

    {clk_prepare,        &g_ckgen_uuu_cpu1a_div_m_pre},
    {clk_prepare,        &g_ckgen_uuu_cpu1a_div_n_pre},
    {clk_prepare,        &g_ckgen_uuu_cpu1a_div_p_pre},
    {clk_prepare,        &g_ckgen_uuu_cpu1a_div_q_pre},
    {clk_prepare,        &g_ckgen_uuu_cpu1b_div_m_pre},
    {clk_prepare,        &g_ckgen_uuu_cpu1b_div_n_pre},
    {clk_prepare,        &g_ckgen_uuu_cpu1b_div_p_pre},
    {clk_prepare,        &g_ckgen_uuu_cpu1b_div_q_pre},

    {clk_prepare,        &g_pll_cpu1a_vco},
    {clk_prepare,        &g_pll_cpu1a_root},
    {clk_prepare,        &g_pll_cpu1a_div_a},
    {clk_prepare,        &g_pll_cpu1a_div_b},
    {clk_prepare,        &g_pll_cpu1b_vco},
    {clk_prepare,        &g_pll_cpu1b_root},
    {clk_prepare,        &g_pll_cpu1b_div_a},
    {clk_prepare,        &g_pll_cpu1b_div_b},

    {clk_prepare,        &g_ckgen_uuu_cpu1a_div_m},
    {clk_prepare,        &g_ckgen_uuu_cpu1a_div_n},
    {clk_prepare,        &g_ckgen_uuu_cpu1a_div_p},
    {clk_prepare,        &g_ckgen_uuu_cpu1a_div_q},
    {clk_prepare,        &g_ckgen_uuu_cpu1b_div_m},
    {clk_prepare,        &g_ckgen_uuu_cpu1b_div_n},
    {clk_prepare,        &g_ckgen_uuu_cpu1b_div_p},
    {clk_prepare,        &g_ckgen_uuu_cpu1b_div_q},

    {clk_enable,         &g_ckgen_gating_cpu1_clk0},
    {clk_enable,         &g_ckgen_gating_cpu1_clk1},
    {clk_enable,         &g_ckgen_gating_cpu1_clk2},
    {clk_enable,         &g_ckgen_gating_cpu1_clk3},
    {clk_enable,         &g_ckgen_gating_cpu1_clk4},
    {clk_enable,         &g_ckgen_gating_cpu1_clk5},
    {NULL, NULL},
};

static scr_signal_t ap1_scr_signal[] = {
     SCR_SEC__cpu1_rvbaraddr0_29_2,
     SCR_SEC__cpu1_rvbaraddr0_39_30,
     SCR_SEC__cpu1_rvbaraddr1_29_2,
     SCR_SEC__cpu1_rvbaraddr1_39_30,
     SCR_SEC__cpu1_rvbaraddr2_29_2,
     SCR_SEC__cpu1_rvbaraddr2_39_30,
     SCR_SEC__cpu1_rvbaraddr3_29_2,
     SCR_SEC__cpu1_rvbaraddr3_39_30
};

static uint32_t ap1_scr_value[] = {
    (CPU1_ENTRY & BIT_MASK(30)) >> 2,   // 0x5e80000,
    (CPU1_ENTRY >> 30)                  // 0x1
};

static struct reboot_cfg reboot_ap1 = {
    .id = CPU_ID_CPU1,
    .seeker_base = IMG_SEEK_BASE,
    .seeker_size = IMG_SEEK_SIZE,
    .pt_name = "preloader",
    .rst_cpu = &rstsig_cpu1_all,
    .scr_entry_signal = {
        .scr_dev = &g_scr_sec,
        .scr_sig = ap1_scr_signal,
        .scr_value = ap1_scr_value,
        .num = ARRAY_SIZE(ap1_scr_signal),
    },
    .target_addr = CPU1_MEMBASE,
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
    .pt_name = "cluster_preloader",
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

int32_t boot_mp()
{
    reboot_ctrl(&reboot_mp);
    return 0;
}

int32_t boot_ap1()
{
    int32_t cnt = 0;

    // module reset
    while (s_ap1_pre_rst_arry[cnt].rst_fun != NULL) {
        s_ap1_pre_rst_arry[cnt].rst_fun(s_ap1_pre_rst_arry[cnt].rst_sig);
        cnt++;
    }

    // clock prepare & enable
    cnt = 0;
    while (s_ap1_clk_arry[cnt].clk_fun != NULL) {
        s_ap1_clk_arry[cnt].clk_fun(s_ap1_clk_arry[cnt].clk);
        cnt++;
    }

    // post module reset
    cnt = 0;
    while (s_ap1_post_rst_arry[cnt].rst_fun != NULL) {
        s_ap1_post_rst_arry[cnt].rst_fun(s_ap1_post_rst_arry[cnt].rst_sig);
        cnt++;
    }

    reboot_ctrl(&reboot_ap1);
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
    boot_ap1();
#ifdef CONFIG_KUNLUN_BOOT_MP
    boot_mp();
#endif
}

#endif
