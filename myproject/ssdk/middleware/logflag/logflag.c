/*
 * logflag.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 */
#include <arch.h>
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regs_base.h>
#include <reg.h>
#include "logflag.h"
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

static void sdrv_reg_write(long reg, int shift, int mask, int val)
{
    long addr;
    int tmp;
#if WITH_KERNEL_VM
    addr = (long)paddr_to_kvaddr(reg);
#else
    addr = reg;
#endif
    tmp = readl(addr);
    tmp &= ~(mask << shift);
    tmp |= (val & mask) << shift;
    writel(tmp, addr);
}

static int sdrv_reg_read(long reg, int shift, int mask)
{
    long addr;
    int tmp;
#if WITH_KERNEL_VM
    addr = (long)paddr_to_kvaddr(reg);
#else
    addr = reg;
#endif
    tmp = readl(addr);
    return (tmp >> shift) & mask;
}

int sdrv_exp_rst_cnt_get(flag_id_t id)
{
    if (id >= FLAG_ID_MAX)
        return 0;

    return sdrv_reg_read(EXP_LOG_STATE_REG,
                         id * EXP_LOG_RST_CNT_BITS,
                         EXP_LOG_RST_CNT_MASK);
}

void sdrv_exp_rst_cnt_set(flag_id_t id, int val)
{
    if (id >= FLAG_ID_MAX)
        return;

    sdrv_reg_write(EXP_LOG_STATE_REG,
                   id * EXP_LOG_RST_CNT_BITS,
                   EXP_LOG_RST_CNT_MASK, val);
}

void sdrv_exp_rst_cnt_add(flag_id_t id, int val)
{
    int cnt;

    if (id >= FLAG_ID_MAX)
        return;

    cnt = sdrv_exp_rst_cnt_get(id);
    sdrv_exp_rst_cnt_set(id, cnt + val);
}

int sdrv_is_loop_exp_restart(flag_id_t id)
{
    int cnt;

    if (id >= FLAG_ID_MAX)
        return 0;

    cnt = sdrv_exp_rst_cnt_get(id);

    if (cnt >= EXP_LOG_CYCLE_RESTART)
        return 1;

    return 0;
}

void sdrv_global_exp_flag_set(int val)
{
    sdrv_reg_write(EXP_LOG_STATE_REG,
                   EXP_LOG_GLOBAL_FLAG_SHIFT,
                   EXP_LOG_GLOBAL_FLAG_MASK, val);
}

void sdrv_ap_exp_flag_set(int val)
{
    sdrv_reg_write(EXP_LOG_STATE_REG,
                   EXP_LOG_AP_EXP_FLAG_SHIFT,
                   EXP_LOG_AP_EXP_FLAG_MASK, val);
}

int sdrv_ap_exp_flag_get(void)
{
    return sdrv_reg_read(EXP_LOG_STATE_REG,
                         EXP_LOG_AP_EXP_FLAG_SHIFT,
                         EXP_LOG_AP_EXP_FLAG_MASK);
}

void sdrv_ap2_exp_flag_set(int val)
{
    sdrv_reg_write(EXP_LOG_STATE_REG,
                   EXP_AP2_EXP_FLAG_SHIFT,
                   EXP_AP2_EXP_FLAG_MASK, val);
}

int sdrv_ap2_exp_flag_get(void)
{
    return sdrv_reg_read(EXP_LOG_STATE_REG,
                         EXP_AP2_EXP_FLAG_SHIFT,
                         EXP_AP2_EXP_FLAG_MASK);
}

void sdrv_crashdump_flag_set(int val)
{
    sdrv_reg_write(EXP_LOG_STATE_REG,
                   EXP_CRASHDUMP_FLAG_SHIFT,
                   EXP_CRASHDUMP_FLAG_MASK, val);
}

int sdrv_crashdump_flag_get(void)
{
    return sdrv_reg_read(EXP_LOG_STATE_REG,
                         EXP_CRASHDUMP_FLAG_SHIFT,
                         EXP_CRASHDUMP_FLAG_MASK);
}

void sdrv_saf_uart_flag_set(int val)
{
    sdrv_reg_write(EXP_LOG_STATE_REG,
                   EXP_SAF_UART_FLAG_SHIFT,
                   EXP_SAF_UART_FLAG_MASK, val);
}

int sdrv_saf_uart_flag_get(void)
{
    return sdrv_reg_read(EXP_LOG_STATE_REG,
                         EXP_SAF_UART_FLAG_SHIFT,
                         EXP_SAF_UART_FLAG_MASK);
}
