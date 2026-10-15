/**
 * @file boot_core.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#include <stdlib.h>
#include <stdio.h>
#include <types.h>
#include <param.h>
#include <string.h>

#include <regs_base.h>
#include "rstgen_cfg.h"
#include "scr_hw.h"
#include "scr_cfg.h"

#ifdef CONFIG_CLI
#include <CLI.h>
#endif

#define SP0_MEM_BASE 0x700000
#define SP1_MEM_BASE 0x780000
#define SX0_MEM_BASE 0x600000
#define SX1_MEM_BASE 0x680000

typedef enum {
    SF = 0,
    SP0,
    SP1,
    SX0,
    SX1,
    SP,
    SX,
    MAX_CORE_ID,
} boot_core_id_t;

static scr_signal_t sp_split_scr[] = {
    SCR_SF_VIC2_IS_LOCKSTEP_MODE_P,
    SCR_SF_SCR_CR5_SP_LS_DISABLE,
    SCR_SF_SCR_CR5_SP_SLCLAMP_DISABLE,
};

static scr_signal_t sx_split_scr[] = {
    SCR_SF_VIC3_IS_LOCKSTEP_MODE_P,
    SCR_SF_SCR_CR5_SX_LS_DISABLE,
    SCR_SF_SCR_CR5_SX_SLCLAMP_DISABLE,
};

static scr_signal_t sp0_remap_scr[] = {
    SCR_SF_REMAP_CR5_SP_AR0_ADDR_OFFSET_19_0,
    SCR_SF_REMAP_CR5_SP_AR0_REMAP_EN,
    SCR_SF_REMAP_CR5_SP_AW0_ADDR_OFFSET_19_0,
    SCR_SF_REMAP_CR5_SP_AW0_REMAP_EN,
};

static scr_signal_t sp1_remap_scr[] = {
    SCR_SF_REMAP_CR5_SP_AR1_ADDR_OFFSET_19_0,
    SCR_SF_REMAP_CR5_SP_AR1_REMAP_EN,
    SCR_SF_REMAP_CR5_SP_AW1_ADDR_OFFSET_19_0,
    SCR_SF_REMAP_CR5_SP_AW1_REMAP_EN,
};

static scr_signal_t sx0_remap_scr[] = {
    SCR_SF_REMAP_CR5_SX_AR0_ADDR_OFFSET_19_0,
    SCR_SF_REMAP_CR5_SX_AR0_REMAP_EN,
    SCR_SF_REMAP_CR5_SX_AW0_ADDR_OFFSET_19_0,
    SCR_SF_REMAP_CR5_SX_AW0_REMAP_EN,
};

static scr_signal_t sx1_remap_scr[] = {
    SCR_SF_REMAP_CR5_SX_AR1_ADDR_OFFSET_19_0,
    SCR_SF_REMAP_CR5_SX_AR1_REMAP_EN,
    SCR_SF_REMAP_CR5_SX_AW1_ADDR_OFFSET_19_0,
    SCR_SF_REMAP_CR5_SX_AW1_REMAP_EN,
};

static void sp_split_mode(bool split)
{
    for (int i = 0; i < 3; i++) {
        scr_set(&g_scr_sf, &sp_split_scr[i], split);
    }
}

static void sx_split_mode(bool split)
{
    for (int i = 0; i < 3; i++) {
        scr_set(&g_scr_sf, &sx_split_scr[i], split);
    }
}

static void remap_sp0_core(paddr_t addr)
{
    scr_set(&g_scr_sf, &sp0_remap_scr[0], addr >> 12);
    scr_set(&g_scr_sf, &sp0_remap_scr[1], 1);
    scr_set(&g_scr_sf, &sp0_remap_scr[2], addr >> 12);
    scr_set(&g_scr_sf, &sp0_remap_scr[3], 1);
}

static void remap_sp1_core(paddr_t addr)
{
    scr_set(&g_scr_sf, &sp1_remap_scr[0], addr >> 12);
    scr_set(&g_scr_sf, &sp1_remap_scr[1], 1);
    scr_set(&g_scr_sf, &sp1_remap_scr[2], addr >> 12);
    scr_set(&g_scr_sf, &sp1_remap_scr[3], 1);
}

static void remap_sx0_core(paddr_t addr)
{
    scr_set(&g_scr_sf, &sx0_remap_scr[0], addr >> 12);
    scr_set(&g_scr_sf, &sx0_remap_scr[1], 1);
    scr_set(&g_scr_sf, &sx0_remap_scr[2], addr >> 12);
    scr_set(&g_scr_sf, &sx0_remap_scr[3], 1);
}

static void remap_sx1_core(paddr_t addr)
{
    scr_set(&g_scr_sf, &sx1_remap_scr[0], addr >> 12);
    scr_set(&g_scr_sf, &sx1_remap_scr[1], 1);
    scr_set(&g_scr_sf, &sx1_remap_scr[2], addr >> 12);
    scr_set(&g_scr_sf, &sx1_remap_scr[3], 1);
}

static bool is_valid_core_id(uint8_t core_id)
{
    if (core_id < SF || core_id >= MAX_CORE_ID)
        return false;

    return true;
}

static uint8_t get_core_id(const char* core)
{
    uint8_t core_id = MAX_CORE_ID;
    const char * core_list[] = {"sf", "sp0", "sp1", "sx0", "sx1", "sp", "sx"};

    for(uint32_t i = 0 ;i < ARRAY_SIZE(core_list); i++)
    {
        if (!strcmp(core, core_list[i])){
            core_id = i;
            goto out;
        }
    }

    /* Here, core is digatal */
    if (core[0] >= '0' && core[0] <= '9')
        core_id = atoi(core);

out:
    return core_id;
}

static int sf_boot_core(int argc, char *argv[])
{
    uint8_t core_id = 0;
    paddr_t addr = 0;

    if (argc > 0) {
        core_id = get_core_id(argv[0]);
    }

    if (argc > 1) {
        addr = strtoul(argv[1], NULL, 16);
        if ((addr & 0xFFF) != 0)
        {
            printf("ERROR:addr 0x%0x is not 4KB aligned\n", addr);
            return 0;
        }
    }

    if (!is_valid_core_id(core_id)){
        printf("ERROR:invalid core id:%s\n", argv[0]);
        return 0;
    }

    if (core_id == SF) {
        reset_ctl_reset(&rstsig_cr5_saf);
    }
    else if (core_id == SP) {
        sp_split_mode(false);

        if (addr < 0x1000) {
            addr = SP0_MEM_BASE;
        }

        remap_sp0_core(addr);
        reset_ctl_reset(&rstsig_cr5_sp0);
    }
    else if (core_id == SP0) {
        sp_split_mode(true);

        if (addr < 0x1000) {
            addr = SP0_MEM_BASE;
        }

        remap_sp0_core(addr);
        reset_ctl_reset(&rstsig_cr5_sp0);
    }
    else if (core_id == SP1) {
        sp_split_mode(true);

        if (addr < 0x1000) {
            addr = SP1_MEM_BASE;
        }

        remap_sp1_core(addr);
        reset_ctl_reset(&rstsig_cr5_sp1);
    }
    else if (core_id == SX) {
        sx_split_mode(false);

        if (addr < 0x1000) {
            addr = SX0_MEM_BASE;
        }

        remap_sx0_core(addr);
        reset_ctl_reset(&rstsig_cr5_sx0);
    }
    else if (core_id == SX0) {
        sx_split_mode(true);

        if (addr < 0x1000) {
            addr = SX0_MEM_BASE;
        }

        remap_sx0_core(addr);
        reset_ctl_reset(&rstsig_cr5_sx0);
    }
    else if (core_id == SX1) {
        sx_split_mode(true);

        if (addr < 0x1000) {
            addr = SX1_MEM_BASE;
        }

        remap_sx1_core(addr);
        reset_ctl_reset(&rstsig_cr5_sx1);
    }

    return 0;
}

CLI_CMD("boot",
        "\r\nboot <cpu_id> <base_addr>:\r\n    0  SF\r\n    1  SP0\r\n    2  SP1\r\n    3  SX0\r\n    4  SX1\r\n    5  SP\r\n    6  SX\r\n",
        sf_boot_core);