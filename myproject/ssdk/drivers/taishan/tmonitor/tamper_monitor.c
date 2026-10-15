/*
 * tamper_ip.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: tamper monitor driver.
 *
 * Revision History:
 * -----------------
 */
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <reg.h>
#include <stdint.h>
#include <types.h>

#include "tamper_monitor.h"

void tm_out_ej(uint8_t type, uint8_t en)
{
    uint32_t v = TMONITOR->TM_OUT_ERR_INJ;
    uint32_t wdata = 0;
    switch (type) {
    case TAMPER_VIO_EJ_ONDIE:
        wdata = BM_TM_OUT_ERR_INJ_ON_DIE_VIOLATION_ERR_INJ_EN |
                BM_TM_OUT_ERR_INJ_ON_DIE_VIOLATION_ERR_INJ;
        break;
    case TAMPER_VIO_EJ_PIN:
        wdata = BM_TM_OUT_ERR_INJ_PIN_VIOLATION_ERR_INJ_EN |
                BM_TM_OUT_ERR_INJ_PIN_VIOLATION_ERR_INJ;
        break;
    case TAMPER_VIO_EJ_CLOCK:
        wdata = BM_TM_OUT_ERR_INJ_CLOCK_VIOLATION_ERR_INJ_EN |
                BM_TM_OUT_ERR_INJ_CLOCK_VIOLATION_ERR_INJ;
        break;
    }
    if (en)
        v |= wdata;
    else
        v &= ~wdata;
    TMONITOR->TM_OUT_ERR_INJ = v;
}
void tm_out_ej_cancel(uint8_t type)
{
    uint32_t v = TMONITOR->TM_OUT_ERR_INJ;
    v &= ~(1 << (type * 2));
    TMONITOR->TM_OUT_ERR_INJ = v;
}
void tm_ej_en(uint8_t en)
{
    uint32_t v = 0;
    if (en)
        v = BM_TAMPER_MONITOR_EJ_EN_GLOBAL_EJ_EN;
    TMONITOR->TAMPER_MONITOR_EJ_EN = v;
}

uint8_t tm_chk_out_ej(void)
{
    uint32_t v = TMONITOR->TM_OUT_ERR_INJ;
    uint8_t i = 0;
    for (i = 0; i <= TAMPER_VIO_EJ_CLOCK; i++)
        if (v & (3 << (2 * i)))
            return i;
    return TAMPER_VIO_EJ_NULL;
}