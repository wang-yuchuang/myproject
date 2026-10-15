/*
 * sem_monitor.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: SEM error monitor.
 *
 * Revision History:
 * -----------------
 */
#include <stdio.h>
#include <stdlib.h>
#ifdef CONFIG_SYS_DIAGNOSIS
#include <sys_diagnosis.h>
#endif
#include <cmsis_os2.h>
#include "CLI_type.h"
#include <reg.h>
#include <board.h>
#include <debug.h>

/*
 * NOTE: GPV_SAF_BASE is different for AP and safety domain, but
 * APB_GPV_SAF_BASE is invalid in safety __regs_base.h.
 */
#define SAF_SAF_GPV_BASE    0xF4200000

static volatile bool g_enable_sem_monitor;


static void sem_clear_error(uint32_t addr)
{
    uint32_t val;

    RMWREG32(addr + 0x2c, 1, 1, 0x0);
    writel(0x3, addr + 0x30);

    RMWREG32(addr + 0x3c, 0, 16, 0x3000);   /* set bisttimeout1 reg */
    RMWREG32(addr + 0x40, 0, 8, 0xFF);      /* set bisttimeout2 reg */
    RMWREG32(addr + 0x34, 0, 1, 0x1);       /* start bist seq */

    /* polling bistdone stat */
    do {
        val = readl(addr + 0x38);
    } while (!(val & 0x1));

    RMWREG32(addr + 0x34, 1, 1, 0x1);   /* clear bistdone */
    RMWREG32(addr + 0x30, 0, 2, 0x3);   /* clear latent int */
}
#ifdef CONFIG_SYS_DIAGNOSIS
static int sem_monitor(int signal,void* args)
{

    printf("SEM signal %d toggled!\n", signal);

    switch (signal) {
        case NOC_SEC____PD_SEC_mainMissionInt:
            sem_clear_error(APB_GPV_SEC_BASE);
            break;
#ifdef CONFIG_BOARD_X9SP
        case NOC_MAINW_____SOC_mainMissionIntBar:
            sem_clear_error(APB_GPV_HPIB_BASE + 0x2880);
#else
        case NOC_MAIN_____SOC_mainMissionIntBar:
            sem_clear_error(APB_GPV_HPIA_BASE + 0x2880);
#endif
            break;

        case NOC_SAF____PD_SAF_mainMissionInt:
            sem_clear_error(SAF_SAF_GPV_BASE);
            break;

        default:
            break;
    }

    return 0;
}
#endif
static int sem_monitor_on(int argc, char *argv[])
{
    if (argc != 1) {
        ssdk_printf(SSDK_CRIT, "input argv error\n");
        ssdk_printf(SSDK_CRIT, "sem_on 1 or sem_on 0\n");
        return CLI_FALSE;
    }

    static bool registered = false;
    uint32_t value = strtoul(argv[0], NULL, 10);
    printf("%s: sem_on %d\n", __func__, value);

    if(value == 1)
    {
        if(!registered)
        {
            registered = true;
#ifdef CONFIG_SYS_DIAGNOSIS
            sysd_register_handler(sem_monitor,NULL,3,
            NOC_SEC____PD_SEC_mainMissionInt,
#ifdef CONFIG_BOARD_X9SP
            NOC_MAINW_____SOC_mainMissionIntBar,
#else /* CONFIG_BOARD_X9SP */
            NOC_MAIN_____SOC_mainMissionIntBar,
#endif /* CONFIG_BOARD_X9SP */
            NOC_SAF____PD_SAF_mainMissionInt);
#else /* CONFIG_SYS_DIAGNOSIS */
            printf("This core does not support system diagnosis.\n");
#endif /* CONFIG_SYS_DIAGNOSIS */
        }

    }

    return CLI_FALSE;
}

CLI_CMD("sem_on", "sem_on 1 or sem_on 0", sem_monitor_on);

