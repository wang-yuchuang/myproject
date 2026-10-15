
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_SDRAMC

#include <debug.h>
#include <driver/taishan/sdramc/sdramc.h>
#include <regs_base.h>
#include <types.h>
#include <reg.h>
#include "clock_cfg.h"
#include <param.h>

static sdram_config_t sdram_config_array[] = {
    [0] = {
        .name = "Z1_ddr_x4",
        .ddr_mode = true,
        .row_num = 4096,
        .column_num = 256,
        .bank_num = 4,
        .cs_num = 4,
        .dq_width = 32,
        .clock = 400000000,
        .cas_latency_x2 = 6,
        .burst_length = 2,
    },
    [1] = {
        .name = "IS45_sdr_x1",
        .ddr_mode = false,
        .row_num = 8192,
        .column_num = 512,
        .bank_num = 4,
        .cs_num = 1,
        .dq_width = 32,
        .clock = 143000000,
        .cas_latency_x2 = 6,
        .burst_length = 2,
    },
};

sdram_comm_config_t sdram_comm_config = {
    .mrd_cycle = 2,
    .t_wtr_cycle = 2,
    .t_rcd_ns = 18,
    .t_rfc_ns = 70,
    .t_rc_ns = 55,
    .t_ras_ns = 42,
    .t_wr_ns = 15,
    .t_rrd_ns = 12,
    .t_rp_ns = 18,
};

void board_sdramc_init(void)
{

#ifdef CONFIG_WORK_ON_Z1
    int index = 0;
#else
    int index = 1;
#endif

    addr_t sdramc_base = APB_SDRAMC_BASE;
    sdram_config_t  *sdramc_cfg = &sdram_config_array[index];
    sdramc_cfg->comm_config = &sdram_comm_config;

    clk_node_t *clk = CLK_NODE(g_ckgen_ip_sdramc);
    SDRAMC_CTL_DATA_DEBUG("sdramc_cfg default config nd clk = %d \n",
                          clk_get_rate(clk));
    SDRAMC_CTL_DATA_DEBUG("sdramc_cfg parent clk = %d \n",
                          clk_get_rate(clk->parents[clk->mux]));
    SDRAMC_CTL_DATA_DEBUG("sdramc_cfg want to set clock = %d \n",
                          sdramc_cfg->clock);

    if (clk_set_rate(clk, MIN(clk_get_rate(clk->parents[clk->mux]),
                              sdramc_cfg->clock)) < 0) {
        ssdk_printf(SSDK_CRIT, "clk_set_rate failed \n");
        return;
    }

    sdramc_cfg->clock = clk_get_rate(clk);
    ssdk_printf(SSDK_CRIT, "sdramc_cfg final set clock = %d \n", sdramc_cfg->clock);

    SDRAMC_CTL_DATA_DEBUG("sdramc clock %d! dq_width %d! \n", sdramc_cfg->clock,
                          sdramc_cfg->dq_width);
    SDRAMC_CTL_DATA_DEBUG("burst_length is %d! cas_latency is %d!\n",
                          sdramc_cfg->burst_length, sdramc_cfg->cas_latency_x2);
    SDRAMC_CTL_DATA_DEBUG("row_num is %d! column_num is %d! \ncs_num  %d!\n",
                          sdramc_cfg->row_num, sdramc_cfg->column_num, sdramc_cfg->cs_num);

    if (sdramc_init(sdramc_base, sdramc_cfg)) {
        ssdk_printf(SSDK_ERR, "sdramc init failed!\n");
    }

    SDRAMC_CTL_DATA_DEBUG("r(0x%x, r(0x%08x)\n", sdramc_base + 0xc0,
                          readl(sdramc_base + 0xc0));

    for (int32_t i = 0; i < 4u; i++) {
        SDRAMC_CTL_DATA_DEBUG("r(0x%x, r(0x%08x)\n", sdramc_base + 0x98 + i * 4,
                              readl(sdramc_base + 0x98 + i * 4));
    }

    ssdk_printf(SSDK_CRIT, "sdramc init end! \n");

}

#endif
