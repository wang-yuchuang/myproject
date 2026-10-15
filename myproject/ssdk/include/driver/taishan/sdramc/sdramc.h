/**
 * @file sdramc.h
 * @brief
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef SDRAMC_H_
#define SDRAMC_H_

#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif

// #define ENABLE_SDRAMC_CTL_DATA_DEBUG
#ifdef ENABLE_SDRAMC_CTL_DATA_DEBUG
#define SDRAMC_CTL_DATA_DEBUG(format, ...) printf(format, ##__VA_ARGS__)
#else
#define SDRAMC_CTL_DATA_DEBUG(format, ...)
#endif

typedef struct sdram_comm_config {
    uint32_t mrd_cycle;
    uint32_t t_wtr_cycle;
    uint32_t t_rcd_ns;
    uint32_t t_rfc_ns;
    uint32_t t_rc_ns;
    uint32_t t_ras_ns;
    uint32_t t_wr_ns;
    uint32_t t_rrd_ns;
    uint32_t t_rp_ns;
} sdram_comm_config_t;

typedef struct sdram_config {
    const char *name;
    bool ddr_mode;
    uint32_t row_num;
    uint32_t column_num;
    uint32_t bank_num;
    uint32_t cs_num;
    uint32_t dq_width;
    uint32_t clock;
    uint32_t cas_latency_x2;
    uint32_t burst_length;
    sdram_comm_config_t  *comm_config;
} sdram_config_t;

typedef struct sd_para {
    uint32_t column_start;
    uint32_t column_bits;
    uint32_t cs_start;
    uint32_t cs_bits;
    uint32_t bank_start;
    uint32_t bank_bits;
    uint32_t row_start;
    uint32_t row_bits;
} sd_para_s;

typedef struct gate_record {
    uint32_t  gate_val;
    uint32_t  gate_index;
} gate_record_s;

extern struct sd_para cs_p;

struct sd_para *get_sd_para(void);
void set_sd_para(struct sd_para *cs);
int sdramc_init(addr_t base, sdram_config_t *conf);

#ifdef __cplusplus
}
#endif
#endif /* SDRAMC_H_ */
