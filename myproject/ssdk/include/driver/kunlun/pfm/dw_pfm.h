/*
* dw_pfm.h
*
* Copyright (c) 2020 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement semidrive fuse drv head file
*
*/
#ifndef __DW_PFM_H__
#define __DW_PFM_H__

#include "__regs_pfm.h"

enum handler_return {
    INT_NO_RESCHEDULE = 0,
    INT_RESCHEDULE,
};

/* DDR_4266 */
#define time_window_per_ms 1066500

typedef void (*pfm_irq_callback)(void);

void pfm_cfg_time_window(uint32_t time);
void pfm_enable_group(uint32_t nr);
uint32_t pfm_group_nr(void);
void pfm_set_axi_id0(uint32_t group, uint32_t id, uint32_t mask);
void pfm_set_axi_id1(uint32_t group, uint32_t id, uint32_t mask);
void pfm_set_cnt_thr(uint32_t group, uint32_t rd_thr, uint32_t wr_thr);
void pfm_set_bcnt_thr(uint32_t group, uint32_t rd_bcnt_thr, uint32_t wr_bcnt_thr);
void pfm_enable_irq_mode(void);
void pfm_enable_counter(void);
void pfm_disable_counter(void);
void pfm_enable_dma_mode(void);
void pfm_enable_read_timeout_irq(void);
void pfm_enable_read_wdg(uint32_t timeout);
void pfm_reg_base_cnt_callback(pfm_irq_callback callback);
#endif

