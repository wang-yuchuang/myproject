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

#include "CLI_type.h"

#define IRAMC_POWER_CFG 0x4

#define SAF_LP_CTL(dom) (0x1008 + dom * 0x8)
#define SAF_RAM_LP_CTL(ram) (0x1060 + ram * 0x4)

enum smc_safety_dom {
    CR5_SF,
    CR5_SP,
    CR5_SX,
    GAMA1,
    SAF_MISC,
};

enum smc_safety_ram {
    CR5_SF_RAM,
    CR5_SP_RAM,
    CR5_SX_RAM,
    GAMA1_RAM,
};

static void iramc_power_gate(paddr_t iramc_base, bool gate)
{
    RMWREG32(iramc_base + IRAMC_POWER_CFG, 12, 1, (uint32_t)gate);
    udelay(1);
    RMWREG32(iramc_base + IRAMC_POWER_CFG, 13, 1, (uint32_t)gate);
}

static void safety_dom_gate(paddr_t smc_base, enum smc_safety_dom dom,
                            bool gate)
{
    if (gate) {
        RMWREG32(smc_base + SAF_LP_CTL(dom), 2, 2, (uint32_t)3);
        udelay(1);
        RMWREG32(smc_base + SAF_LP_CTL(dom), 4, 1, (uint32_t)1);
    } else {
        RMWREG32(smc_base + SAF_LP_CTL(dom), 4, 1, (uint32_t)0);
        udelay(1);
        RMWREG32(smc_base + SAF_LP_CTL(dom), 2, 2, (uint32_t)0);
    }
}

static void safety_ram_gate(paddr_t smc_base, enum smc_safety_ram ram,
                            bool gate)
{
    if (gate) {
        RMWREG32(smc_base + SAF_RAM_LP_CTL(ram), 2, 1, (uint32_t)1);
        udelay(1);
        RMWREG32(smc_base + SAF_RAM_LP_CTL(ram), 9, 3, (uint32_t)7);
    } else {
        RMWREG32(smc_base + SAF_RAM_LP_CTL(ram), 9, 3, (uint32_t)3);
        udelay(1);
        RMWREG32(smc_base + SAF_RAM_LP_CTL(ram), 2, 1, (uint32_t)0);
    }
}

/**
 * @brief Simple power gate example.
 *
 * This is a simple example to manually power gate SOC modules. Automatic
 * power gating will be added to SSDK PM framework later, based on System
 * Work Mode (SWM).
 */
static int power_gate_main(int argc, char *argv[])
{
    char *dom = argv[0];
    bool gate = !strcmp(argv[1], "off");

    printf("Power %s, gate %d\n", dom, gate);

    if (!strcmp(dom, "SF")) {
        safety_ram_gate(APB_SMC_BASE, CR5_SF_RAM, gate);
        safety_dom_gate(APB_SMC_BASE, CR5_SF, gate);
    } else if (!strcmp(dom, "SP")) {
        safety_ram_gate(APB_SMC_BASE, CR5_SP_RAM, gate);
        safety_dom_gate(APB_SMC_BASE, CR5_SP, gate);
    } else if (!strcmp(dom, "SX")) {
        safety_ram_gate(APB_SMC_BASE, CR5_SX_RAM, gate);
        safety_dom_gate(APB_SMC_BASE, CR5_SX, gate);
    } else if (!strcmp(dom, "GAMA1")) {
        safety_ram_gate(APB_SMC_BASE, GAMA1_RAM, gate);
        safety_dom_gate(APB_SMC_BASE, GAMA1, gate);
    }

    return CLI_FALSE;
}

CLI_CMD("power_gate", "\r\npower_gate:\r\n E3 Power Gate Example\n",
        power_gate_main);
