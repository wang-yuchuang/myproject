/* __regs_sd_ddr_pfmon.h
 *
 * Copyright (c) 2018 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: implement semidrive pfm driver
 *
 **/

#ifndef __SD_DDR_PFMON_H
#define __SD_DDR_PFMON_H

#ifdef CONFIG_ARCH_CHIP_X9_THANG
#include "pfm_axi_id_thang.h"
#else
#include "pfm_axi_id_kunlun.h"
#endif

#if 0
#ifdef DDR_2133
#define time_window_per_ms 533250
#elif DDR_4266
#define time_window_per_ms 1066500
#endif
#endif

#define PFM_RECORD_ON_DDR 0

#if PFM_RECORD_ON_DDR

#define PFM_RECORD_MAGIC    0x5a5a5a5a
#ifdef CONFIG_ARCH_CHIP_X9_THANG
#define PFM_RECORD_MEM_ADDR    0x73200000
#else
#define PFM_RECORD_MEM_ADDR    0x33200000
#endif
#define PFM_MON_HEAD_OFFSET    0x1000
#define PFM_RECORD_BUFFER    (vaddr_t)_ioaddr(PFM_RECORD_MEM_ADDR)
#define PFM_RECORD_BUFFER_SIZE  3*1024*1024

#endif

typedef struct pfm_observer{
    uint32_t master0;
    uint32_t msk0;
    uint32_t master1;
    uint32_t msk1;
} pfm_observer_t;

typedef struct pfm_stop_condition {
    uint32_t rd_thr;
    uint32_t wr_thr;
    uint32_t rd_bcnt_thr;
    uint32_t wr_bcnt_thr;
} pfm_stop_condition_t;

typedef struct pfm_head {
    uint32_t magic;
    uint32_t start_time;
    uint32_t time_window;
    uint32_t group_nr;
    pfm_observer_t observer_config[16];
    pfm_stop_condition_t condition[8];
    uint32_t mode;
    uint32_t rounds;
    uint32_t pool_size;
    uint32_t record_offset;
    uint32_t reach_thr;
} pfm_head_t;

struct pfm_record_irq {
    uint32_t rd_cnt;
    uint32_t wr_cnt;
    uint32_t rd_bcntl;
    uint32_t rd_bcnth;
    uint32_t wr_bcntl;
    uint32_t wr_bcnth;
};

void start_pfm_service(void);

#endif
