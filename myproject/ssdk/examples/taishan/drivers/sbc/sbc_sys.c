/*
 * sbc_sys.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: SBC System level Driver For SSDK and other operate SBC API.
 *
 * Revision History:
 * -----------------
 */

#include "sbc_sys.h"

static osThreadId_t wtd_server_id;
static uint32_t wtd_period;
static bool wdt_running;

static void _watchdog_Trigger(void *argument)
{
    while (1) {
        if (wdt_running) {
            SBC_WD_Trigger();
        }

        osDelay(wtd_period);
    }

    osThreadExit();
}

osThreadId_t SBC_Create_WD_Trigger_Task(uint32_t period)
{
    osThreadAttr_t sbc_wd_attr = {
        .name = "sbc_watchdog_Trigger",
        .stack_size = 4096,
        .priority = osPriorityRealtime7,
    };

    wdt_running = true;

    wtd_server_id = osThreadNew(_watchdog_Trigger, NULL, &sbc_wd_attr);
    wtd_period = period;

    return wtd_server_id;
}

void SBC_Suspend_WD_Trigger_Task(void)
{
    if (wtd_server_id) {
        osThreadTerminate(wtd_server_id);
    }
}

void SBC_Set_WD_Trigger_Period(uint32_t period)
{
    wtd_period = period;
}

uint32_t SBC_Get_WD_Trigger_Period(void)
{
    return wtd_period;
}

SBC_ErrorCode SBC_Soft_Reset(void) {
    return SBC_Write_RegField(SBC_M_S_CTRL, SBC_M_S_CTRL_MODE_Msk, SBC_M_S_CTRL_MODE_Pos, SBC_MODE_RESET, NULL);
}

void SBC_Mode_Stop_Without_WTD(void) {
    uint16_t rltvalue = 0;

    wdt_running = false;

    SBC_Write_RegField(SBC_WK_CTRL_0,SBC_WK_CTRL_0_WD_STM_EN_1_Msk,SBC_WK_CTRL_0_WD_STM_EN_1_Pos,1,NULL);

    uint8_t WD_CTRL = SBC_Read_Command(SBC_WD_CTRL);
    WD_CTRL |= 0x40;
    uint8_t WD_Checksum = WD_CTRL;

    /* Calculate checksum */
    WD_Checksum = WD_Checksum ^ WD_Checksum >> 4;
    WD_Checksum = WD_Checksum ^ WD_Checksum >> 2;
    WD_Checksum = WD_Checksum ^ WD_Checksum >> 1;

    if((WD_Checksum & 1) > 0) {
        /* Set parity bit */
        WD_CTRL = WD_CTRL | 0x80U;
    }

    SBC_Write_Reg(SBC_WD_CTRL, WD_CTRL, &rltvalue);

    SBC_Mode_Stop();
}

void SBC_Set_Mode_Normal(void)
{
    SBC_Mode_Normal();
    SBC_WD_Trigger();
    wdt_running = true;
}
