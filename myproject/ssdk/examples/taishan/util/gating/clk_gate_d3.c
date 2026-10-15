/*
 * power_monitor.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Board level monitor monitor example.
 *
 * Revision History:
 * -----------------
 */

#include <board.h>
#include <param.h>
#include <reg.h>
#include <stdio.h>
#include <string.h>

#include "clock_cfg.h"
#include "CLI_type.h"

/**
 * @brief Simple clock gate example.
 */
static int clock_gate_main(int argc, char *argv[])
{
    char *dom = argv[0];
    bool gate = !strcmp(argv[1], "off");
    int (*fn)(clk_node_t *);

    printf("Clock %s, gate %d\n", dom, gate);
    fn = gate? clk_disable : clk_enable;

    if (!strcmp(dom, "SF")) {
        fn(CLK_NODE(g_ckgen_gating_cr5_sf_clkin));
    } else if (!strcmp(dom, "SP")) {
        fn(CLK_NODE(g_ckgen_gating_cr5_sp0_clkin));
        fn(CLK_NODE(g_ckgen_gating_cr5_sp1_clkin));
    } else if (!strcmp(dom, "SX")) {
        fn(CLK_NODE(g_ckgen_gating_cr5_sx0_clkin));
        fn(CLK_NODE(g_ckgen_gating_cr5_sx1_clkin));
    } else if (!strcmp(dom, "GAMA1")) {
        fn(CLK_NODE(g_ckgen_gating_gama1_aclk));
        fn(CLK_NODE(g_ckgen_gating_gama1_hclk));
    } else if (!strcmp(dom, "IRAM1")) {
        fn(CLK_NODE(g_ckgen_gating_iramc1_aclk));
        fn(CLK_NODE(g_ckgen_gating_iramc1_pclk));
    } else if (!strcmp(dom, "IRAM2")) {
        fn(CLK_NODE(g_ckgen_gating_iramc2_aclk));
        fn(CLK_NODE(g_ckgen_gating_iramc2_pclk));
    } else if (!strcmp(dom, "IRAM3")) {
        fn(CLK_NODE(g_ckgen_gating_iramc3_aclk));
        fn(CLK_NODE(g_ckgen_gating_iramc3_pclk));
    } else if (!strcmp(dom, "IRAM4")) {
        fn(CLK_NODE(g_ckgen_gating_iramc4_aclk));
        fn(CLK_NODE(g_ckgen_gating_iramc4_pclk));
    }
    return CLI_FALSE;
}

CLI_CMD("clock_gate", "\r\nclock_gate:\r\n E3 Clock Gate Example\n",
        clock_gate_main);
