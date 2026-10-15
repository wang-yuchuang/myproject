/*
 * @file: sdrv_epwm.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights RESERVEDd.
 *
 * @Description:
 *
 * @Revision History:
 * ----------------
 */

#ifndef DRV_LLD_SDRV_PMU_H
#define DRV_LLD_SDRV_PMU_H

#include <compiler.h>
#include <types.h>

#ifndef LOGI
#define LOGI(fmt, ...)                                                         \
    printf("Info  %d %s %s: " fmt " \n", __LINE__, LOG_TAG, __func__,          \
           ##__VA_ARGS__)
#endif

#ifndef LOGE
#define LOGE(fmt, ...)                                                         \
    printf("Error %d %s %s: " fmt " \n", __LINE__, LOG_TAG, __func__,          \
           ##__VA_ARGS__)
#endif

#ifndef LOGW
#define LOGW(fmt, ...)                                                         \
    printf("Warn  %d %s %s: " fmt " \n", __LINE__, LOG_TAG, __func__,          \
           ##__VA_ARGS__)
#endif

#ifndef PRINT_REG
#define PRINT_REG(reg)                                                         \
    LOGI("#### PRINT_REG %#x : %#x\n", (uint32_t)reg,                          \
         (uint32_t)readl((uint32_t)reg));
#endif

#ifndef ASSEMBLY

__BEGIN_CDECLS

/* pmu status */
typedef enum {
    PMU_PWR_UP,
    PMU_RUN,
    PMU_SLEEP,
    PMU_HIBERNATE,
    PMU_RTC,
    PMU_OFF
} sdrv_pmu_mode_t;

/* pmu external */
typedef enum {
    PMU_PWR_CTRL_0,
    PMU_PWR_CTRL_1,
    PMU_PWR_CTRL_2,
    PMU_PWR_CTRL_3,
} sdrv_pmu_pwrctrl_type;

/**
 * @brief wakup sensitive level.
 */
typedef enum {
    PMU_WAKEUP_LEVEL_SENSITIVE,
    PMU_WAKEUP_EDGE_SENSITIVE
} sdrv_pmu_wakep_sensitive;

typedef enum {
    PMU_WAKEUP_DISABLE,
    PMU_WAKEUP_ENABLE,
} sdrv_pmu_wakep_enable;

typedef struct {
    sdrv_pmu_wakep_enable enable;
    sdrv_pmu_wakep_sensitive sensitive;
    uint8_t
    polarity; /* 0:low or failing edge active， 1:high or rising active */
    uint8_t filterBypass; /* 1:bypass */
} sdrv_pmu_wakeup_chn_t;

typedef struct {
    sdrv_pmu_wakeup_chn_t Chn[2];
    uint8_t wakeupAND;   /* 1:wakup0&wakeup1 both active */
    uint8_t latchWakeup; /* 1:latch wakeup in rtc mode */
    uint8_t internalWakeup;
} sdrv_pmu_wakeup_t;

typedef enum { PMU_BC_ENABLEL, PMU_BC_DISABLE } sdrv_pmu_bc_enable_t;

typedef enum {
    PMU_BC_LEVEL_SENSITIVE,
    PMU_BC_EDGE_SENSITIVE
} sdrv_pmu_bc_sensitive;

typedef struct {
    sdrv_pmu_bc_sensitive sensitive;
    uint8_t polarity;
    uint8_t filterBypass;
    uint8_t windowEnable;
} sdrv_pmu_bc_ctrl_t;

typedef struct {
    uint8_t filter_bypass;
    uint8_t filter_edge;
    uint8_t filter_cnt;
    uint8_t filter_interval;
} sdrv_pmu_filter_t;

typedef struct {
    uint8_t min_on_time;
    uint8_t max_on_time;
    uint8_t min_off_time;
    uint8_t max_off_time;
    uint8_t max_on_enable;
    uint8_t max_off_enable;
} sdrv_pmu_bc_window_t;

typedef enum {
    SF_LV_PG,
    SF_HV_PG,
    AP_LV_PG,
    AP_HV_PG,
    RTC_SF_LV_PG,
    RTC_SF_HV_PG,
    RTC_AP_LV_PG,
    RTC_AP_HV_PG,
} sdrv_pmu_pg_status;

typedef enum { PMU_PG_ENABLE, PMU_PG_DISABLE } sdrv_pmu_pg_ctrl_t;

uint32_t sdrv_pmu_lld_get_cur_mode_status(paddr_t base);
void sdrv_pmu_lld_pwr_on_wdg_en(paddr_t base);

/*********************wakeup ctrl*************************/
void sdrv_pmu_lld_wakeup_init(paddr_t base, const sdrv_pmu_wakeup_t *wakeup);
void sdrv_pmu_lld_wakeup_deinit(paddr_t base);
void sdrv_pmu_lld_wakeup_filter(paddr_t base, uint8_t chn, uint8_t filterCnt,
                                uint8_t SampleInterval);
void sdrv_pmu_lld_bc_wakeup_enable(paddr_t base, sdrv_pmu_wakep_enable enable);
uint32_t sdrv_pmu_lld_get_wakeup_reason(paddr_t base);
uint32_t sdrv_pmu_lld_get_powerdown_reason(paddr_t base);

/*********************button ctrl*************************/
void sdrv_pmu_lld_bc_init(paddr_t base, sdrv_pmu_bc_ctrl_t *wakeup);
void sdrv_pmu_lld_bc_deinit(paddr_t base);
void sdrv_pmu_lld_bc_enbale(paddr_t base);
void sdrv_pmu_lld_bc_max_off_window(paddr_t base, uint8_t time);
void sdrv_pmu_lld_bc_min_off_window(paddr_t base, uint8_t time);
void sdrv_pmu_lld_bc_max_on_window(paddr_t base, uint8_t time);
void sdrv_pmu_lld_bc_min_on_window(paddr_t base, uint8_t time);

void sdrv_pmu_lld_bc_max_off_window_enable(paddr_t base, uint8_t status);
void sdrv_pmu_lld_bc_max_on_window_enable(paddr_t base, uint8_t status);

void sdrv_pmu_lld_pwrdown_request(paddr_t base);
/*********************Power good**************************/
void sdrv_pmu_lld_sf_por_pg_enable(paddr_t base, sdrv_pmu_pg_ctrl_t enable);
void sdrv_pmu_lld_ap_por_pg_enable(paddr_t base, sdrv_pmu_pg_ctrl_t enable);
uint32_t sdrv_pmu_lld_get_pg_status(paddr_t base);

/*********************Power good**************************/
uint32_t sdrv_pmu_lld_set_pwrctrl_enable(paddr_t base, sdrv_pmu_mode_t pmu_mode,
        uint8_t id, uint8_t enable);
uint32_t sdrv_pmu_lld_set_pwrctrl_polarity(paddr_t base, uint8_t id,
        uint8_t polarity);
uint32_t sdrv_pmu_lld_set_pwrctrl_delay(paddr_t base, sdrv_pmu_mode_t pmu_mode,
                                        uint8_t id, uint8_t delay,
                                        uint8_t delay_adj);
uint32_t sdrv_pmu_lld_set_pwrctrl_mode_transtion(paddr_t base,
        sdrv_pmu_mode_t pmu_mode,
        uint8_t id, uint8_t mode);

/*********************Power on****************************/
uint32_t sdrv_pmu_lld_set_pwron_enable(paddr_t base, sdrv_pmu_mode_t pmu_mode,
                                       uint8_t id, uint8_t enable);
uint32_t sdrv_pmu_lld_set_pwron_delay(paddr_t base, sdrv_pmu_mode_t pmu_mode,
                                      uint8_t id, uint8_t delay,
                                      uint8_t delay_adj);
uint32_t sdrv_pmu_lld_set_pwron_transtion_mode(paddr_t base,
        sdrv_pmu_mode_t pmu_mode,
        uint8_t id, uint8_t mode);
uint32_t sdrv_pmu_lld_set_pwron_target(paddr_t base, sdrv_pmu_mode_t pmu_mode,
                                       uint8_t id, uint8_t target);

uint32_t sdrv_pmu_lld_set_power_on_off(paddr_t base, sdrv_pmu_mode_t pmu_mode,
                                       uint32_t domain, bool on_off);

__END_CDECLS

#endif

#endif
