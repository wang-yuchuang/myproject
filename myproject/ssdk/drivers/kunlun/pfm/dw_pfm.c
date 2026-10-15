/*
* dw_pfm.c
*
* Copyright (c) 2022 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement semidrive lk fuse drv
*
*/
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <reg.h>
#include <board.h>
#include <driver/kunlun/pfm/dw_pfm.h>
#include <driver/kunlun/pfm/__regs_pfm.h>
#include "driver/reset.h"
#include "driver/irq.h"
#include "irq_num.h"

typedef struct {
    pfm_irq_callback base_cnt_cb;
    spin_lock_t lock;
}pfm_callbacks;

static pfm_callbacks s_cbs;

/* Time in ms */
void pfm_cfg_time_window(uint32_t time)
{
    writel(time_window_per_ms * time, PFM_MON_BASE_CNT_CMP);
}

void pfm_enable_group(uint32_t nr)
{
    writel(BIT_MASK(nr), PFM_MON_GROUP);
}

uint32_t pfm_group_nr(void)
{
    int i = 0;
    unsigned long ret = readl(PFM_MON_GROUP) & 0xffff;
    for (i = 15; i >= 0; i--)
    {
        if (bitmap_test(&ret, i))
        {
            return i + 1;
        }
    }
    return 0;
}

void pfm_set_axi_id0(uint32_t group, uint32_t id, uint32_t mask)
{
    writel(id, PFM_MON_AXI_ID0(group));
    writel(mask, PFM_MON_AXI_ID0_MSK(group));
}

void pfm_set_axi_id1(uint32_t group, uint32_t id, uint32_t mask)
{
    writel(id, PFM_MON_AXI_ID1(group));
    writel(mask, PFM_MON_AXI_ID1_MSK(group));
}

void pfm_set_cnt_thr(uint32_t group, uint32_t rd_thr, uint32_t wr_thr)
{
    writel(rd_thr, PFM_MON_AXI_RD_THR(group));
    writel(wr_thr, PFM_MON_AXI_WR_THR(group));
}
void pfm_set_bcnt_thr(uint32_t group, uint32_t rd_bcnt_thr, uint32_t wr_bcnt_thr)
{
    writel(rd_bcnt_thr, PFM_MON_AXI_RD_BCNT_THR(group));
    writel(wr_bcnt_thr, PFM_MON_AXI_WR_BCNT_THR(group));
}

void pfm_enable_irq_mode(void)
{
    /* Enable base counter timeout irq && auto restart */
    uint32_t tmp = readl(PFM_MON_AXI_CNT_CTL1);
    tmp |= 2;
    writel(tmp, PFM_MON_AXI_CNT_CTL1);
    /* Enable timeout irq  */
    tmp = readl(PFM_MON_MISC_INT_CTL);
    tmp |= 1;
    writel(tmp, PFM_MON_MISC_INT_CTL);
}

/* Enable pfm */
void pfm_enable_counter(void)
{
    /* Clear base counter */
    writel(2, PFM_MON_BASE_CNT_CTL);
    /* Enable base counter */
    writel(1, PFM_MON_BASE_CNT_CTL);
}
void pfm_disable_counter(void)
{
    writel(0, PFM_MON_BASE_CNT_CTL);
}

void pfm_enable_dma_mode(void)
{
    /* Enable AXI threshold irq, auto restart */
    writel(1, PFM_MON_AXI_CNT_CTL1);
    /* Disable base counter timeout irq */
    uint32_t tmp = readl(PFM_MON_MISC_INT_CTL);
    tmp &= 0;
    writel(tmp, PFM_MON_MISC_INT_CTL);
}

void pfm_enable_read_timeout_irq(void)
{
    uint32_t tmp = readl(PFM_MON_MISC_INT_CTL);
    tmp |= (1<<16);
    writel(tmp, PFM_MON_MISC_INT_CTL);
}
void pfm_enable_read_wdg(uint32_t timeout)
{
    writel((1<<30), PFM_MON_MISC_WDG_CTL);
    writel((1<<31) | timeout, PFM_MON_MISC_WDG_CTL);
}

static int pfm_irq_handler(uint32_t irq, void *arg)
{
    irq_state_t state;
    uint32_t irq_status = readl(PFM_MON_MISC_ST);

    if (irq_status & 0x10) {
        state = spin_lock_irqsave(&s_cbs.lock);
        s_cbs.base_cnt_cb();
        spin_unlock_irqrestore(&s_cbs.lock, state);
	    pfm_enable_counter();
    }

    return INT_NO_RESCHEDULE;
}
static void pfm_register_irq_handle(void)
{
    static int reg_once = 0;

    if (!reg_once) {
        irq_disable(DDR_SS_DDR_SS_INTR_NUM);
	    spin_lock_init(&s_cbs.lock);
        irq_attach(DDR_SS_DDR_SS_INTR_NUM, &pfm_irq_handler, NULL);
        irq_enable(DDR_SS_DDR_SS_INTR_NUM);
	reg_once++;
    }
}

void pfm_reg_base_cnt_callback(pfm_irq_callback callback)
{
    irq_state_t state;

    pfm_register_irq_handle();

    state = spin_lock_irqsave(&s_cbs.lock);
    s_cbs.base_cnt_cb = callback;
    spin_unlock_irqrestore(&s_cbs.lock, state);
}
