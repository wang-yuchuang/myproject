/*
 * @file: sdrv_epwm.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description
 *
 * @Revision History:
 * ----------------
 */
#define LOG_TAG "sdrv_pmu_lld.c "
#include <regs_base.h>
#include <stdio.h>
#include <string.h>
#include <types.h>

#include "sdrv_pmu.h"

#include "cmsis_os2.h"
#include "pmu_reg.h"
#include "reg.h"

uint32_t sdrv_pmu_lld_get_cur_mode_status(paddr_t base)
{
    uint32_t reg = readl(base + PMU_STATE_OFF);
    uint32_t mode_status = FV_PMU_STATE_PMU_STATE(reg);

    return mode_status;
}

void sdrv_pmu_lld_set_pwr_mode(paddr_t base, sdrv_pmu_mode_t mode)
{
    uint8_t cur_mode = sdrv_pmu_lld_get_cur_mode_status(base);
    uint32_t v = readl(base + PG_LP_MODE_CTRL_OFF);

    /* run->sleep */
    if ((PMU_RUN == cur_mode) && (PMU_SLEEP == mode)) {
        v |= FV_PG_LP_MODE_CTRL_RUN_LP_MODE(0x2);
    }
    /* run->hibernate */
    else if ((PMU_RUN == cur_mode) && (PMU_HIBERNATE == mode)) {
        v |= FV_PG_LP_MODE_CTRL_RUN_LP_MODE(0x4);
    }
    /* sleep->run */
    else if ((PMU_SLEEP == cur_mode) && (PMU_RUN == mode)) {
        v |= FV_PG_LP_MODE_CTRL_SLEEP_LP_MODE(0x1);
    }
    /* hibernate->run */
    else if ((PMU_HIBERNATE == cur_mode) && (PMU_RUN == mode)) {
        v |= FV_PG_LP_MODE_CTRL_HIBERNATE_LP_MODE(0x1);
    }
    else {
        /* do nothing */
    }

    writel(v, base + PG_LP_MODE_CTRL_OFF);
}

/******************************************************************************
 * pmu external power ctrl
 *
 *****************************************************************************/
static uint8_t sdrv_pmu_lld_get_ext_pwrCtrl_io_offset(uint8_t id)
{
    uint8_t bit_offset = 0x0u;

    switch (id) {
        case PMU_PWR_CTRL_0:
            bit_offset = 7;
            break;

        case PMU_PWR_CTRL_1:
            bit_offset = 5;
            break;

        case PMU_PWR_CTRL_2:
            bit_offset = 3;
            break;

        case PMU_PWR_CTRL_3:
            bit_offset = 1;
            break;

        default:
            break;
    }

    return bit_offset;
}

static uint8_t sdrv_pmu_lld_get_ext_pwrCtrl_status_offset(uint8_t id)
{
    uint8_t bit_offset = 0x0u;

    switch (id) {
        case PMU_PWR_CTRL_0:
            bit_offset = 6;
            break;

        case PMU_PWR_CTRL_1:
            bit_offset = 4;
            break;

        case PMU_PWR_CTRL_2:
            bit_offset = 2;
            break;

        case PMU_PWR_CTRL_3:
            bit_offset = 0;
            break;

        default:
            break;
    }

    return bit_offset;
}

static uint8_t
sdrv_pmu_lld_get_ext_pwrCtrl_mode_offset(sdrv_pmu_mode_t pmu_mode)
{
    uint8_t mode_offset = 0x0;

    switch (pmu_mode) {
        case PMU_RTC:
            mode_offset = 24;
            break;

        case PMU_RUN:
            mode_offset = 16;
            break;

        case PMU_SLEEP:
            mode_offset = 8;
            break;

        case PMU_HIBERNATE:
            mode_offset = 0;
            break;

        default:
            break;
    }

    return mode_offset;
}

/* external pwr ctrl io status */
/* high level layer need check the mode/id/status range */
/* id 0~3 enable:ON/OFF */
uint32_t sdrv_pmu_lld_set_pwrctrl_enable(paddr_t base, sdrv_pmu_mode_t pmu_mode,
        uint8_t id, uint8_t enable)
{
    uint8_t mode_offset = sdrv_pmu_lld_get_ext_pwrCtrl_mode_offset(pmu_mode);
    uint8_t pwrctrl_offset = sdrv_pmu_lld_get_ext_pwrCtrl_io_offset(id);

    RMWREG32(base + PWR_CTRL_SET0_OFF, mode_offset + pwrctrl_offset, 1, enable);

    return 0;
}

/* mode:auto or manual */
uint32_t sdrv_pmu_lld_set_pwrctrl_mode_transtion(paddr_t base,
        sdrv_pmu_mode_t pmu_mode,
        uint8_t id, uint8_t mode)
{
    uint8_t offset = sdrv_pmu_lld_get_ext_pwrCtrl_mode_offset(pmu_mode);
    uint8_t startbit = sdrv_pmu_lld_get_ext_pwrCtrl_status_offset(id);

    RMWREG32(base + PWR_CTRL_SET0_OFF, offset + startbit, 1, mode);

    return 0;
}

/* ext delay */
uint32_t sdrv_pmu_lld_set_pwrctrl_delay(paddr_t base, sdrv_pmu_mode_t pmu_mode,
                                        uint8_t id, uint8_t delay,
                                        uint8_t delay_adj)
{
    uint8_t mode_offset = PWR_UP_PWR_CTRL_DELAY_OFF + pmu_mode * 4;
    uint8_t pwrctrl_startbit = id * 6;
    uint8_t val = (delay_adj << 4 | delay) & 0x3F;

    RMWREG32(base + mode_offset, pwrctrl_startbit, 6, val);

    return 0;
}

/* ext polarity */
uint32_t sdrv_pmu_lld_set_pwrctrl_polarity(paddr_t base, uint8_t id,
        uint8_t polarity)
{
    uint8_t startbit = 20 + id;

    RMWREG32(base + PWR_CTRL_SET1_OFF, startbit, 1, polarity);

    return 0;
}

/******************************************************************************
 * pmu internal power ctrl
 *
 *****************************************************************************/
static uint8_t
sdrv_pmu_lld_get_internal_pwrCtrl_mode_offset(sdrv_pmu_mode_t pmu_mode)
{
    uint8_t mode_offset = 0x0;

    switch (pmu_mode) {
        case PMU_RUN:
            mode_offset = 0x30;
            break;

        case PMU_SLEEP:
            mode_offset = 0x34;
            break;

        case PMU_HIBERNATE:
            mode_offset = 0x38;
            break;

        case PMU_RTC:
            mode_offset = 0x3C;
            break;

        default:
            break;
    }

    return mode_offset;
}

uint32_t sdrv_pmu_lld_set_pwron_enable(paddr_t base, sdrv_pmu_mode_t pmu_mode,
                                       uint8_t id, uint8_t enable)
{
    uint8_t offset = sdrv_pmu_lld_get_internal_pwrCtrl_mode_offset(pmu_mode);
    /* pwr on/off position */
    uint8_t startbit = id * 4 + 1;

    RMWREG32(base + offset, startbit, 1, enable);

    return 0;
}

/* auto or manual */
uint32_t sdrv_pmu_lld_set_pwron_transtion_mode(paddr_t base,
        sdrv_pmu_mode_t pmu_mode,
        uint8_t id, uint8_t mode)
{
    uint8_t offset = sdrv_pmu_lld_get_internal_pwrCtrl_mode_offset(pmu_mode);
    /* pwr auto/manual position */
    uint8_t startbit = id * 4;

    RMWREG32(base + offset, startbit, 1, mode);

    return 0;
}

/* 00:sf 01:ap 1x:not ap/sf */
uint32_t sdrv_pmu_lld_set_pwron_target(paddr_t base, sdrv_pmu_mode_t pmu_mode,
                                       uint8_t id, uint8_t target)
{
    uint8_t offset = sdrv_pmu_lld_get_internal_pwrCtrl_mode_offset(pmu_mode);
    /* pwr target position */
    uint8_t startbit = id * 4 + 2;
    uint8_t domain = target & 0x3;

    RMWREG32(base + offset, startbit, 2, domain);

    return 0;
}

uint32_t sdrv_pmu_lld_set_power_on_off(paddr_t base, sdrv_pmu_mode_t pmu_mode,
                                       uint32_t domain, bool on_off)
{
    uint8_t offset;
    uint8_t startbit;

    offset = sdrv_pmu_lld_get_internal_pwrCtrl_mode_offset(pmu_mode);

    if (domain == 0) {
        /* SF domain */
        startbit = 17;
    }
    else if (domain == 1) {
        /* AP domain */
        startbit = 16;
    }
    else {
        return 0;
    }

    RMWREG32(base + offset, startbit, 1, on_off & 0x1u);

    return 0;
}

/* interrnal power on delay */
uint32_t sdrv_pmu_lld_set_pwron_delay(paddr_t base, sdrv_pmu_mode_t pmu_mode,
                                      uint8_t id, uint8_t delay,
                                      uint8_t delay_adj)
{
    uint8_t offset = PWR_UP_PWR_ON_DELAY_OFF + pmu_mode * 4;
    uint8_t startbit = id * 6;
    uint8_t val = (delay_adj << 4 | delay) & 0x3F;

    RMWREG32(base + offset, startbit, 6, val);

    return 0;
}

/******************************************************************************
 * pmu wakeup
 *
 *****************************************************************************/
static void sdrv_pmu_lld_wakeup_0_init(const sdrv_pmu_wakeup_chn_t *chn,
                                       uint32_t *v)
{
    /* detect mode */
    if (PMU_WAKEUP_LEVEL_SENSITIVE == chn->sensitive) {
        *v &= ~BM_WAKEUP_CTRL_WKUP0_EDGE;
    }
    else {
        *v |= BM_WAKEUP_CTRL_WKUP0_EDGE;
    }

    /* polarity */
    if (0x1 == chn->polarity) {
        *v &= ~BM_WAKEUP_CTRL_WKUP0_POL;
    }
    else {
        *v |= BM_WAKEUP_CTRL_WKUP0_POL;
    }

    /* filter when in edge sensitive  */
    if (0x0 == chn->filterBypass) {
        *v &= ~BM_WAKEUP_CTRL_WKUP0_FILTER_BYPASS;
        /* TODO same other config need be realize */
    }
    else {
        /* do nothing */
    }

    /* enable wakeup 0 */
    if (PMU_WAKEUP_ENABLE == chn->enable) {
        *v &= ~BM_WAKEUP_CTRL_WKUP0_DISABLE;
    }
    else {
        *v |= BM_WAKEUP_CTRL_WKUP0_DISABLE;
    }
}

static void sdrv_pmu_lld_wakeup_1_init(const sdrv_pmu_wakeup_chn_t *chn,
                                       uint32_t *v)
{
    /* detect mode */
    if (PMU_WAKEUP_LEVEL_SENSITIVE == chn->sensitive) {
        *v &= ~BM_WAKEUP_CTRL_WKUP1_EDGE;
    }
    else {
        *v |= BM_WAKEUP_CTRL_WKUP1_EDGE;
    }

    /* polarity */
    if (0x1 == chn->polarity) {
        *v &= ~BM_WAKEUP_CTRL_WKUP1_POL;
    }
    else {
        *v |= BM_WAKEUP_CTRL_WKUP1_POL;
    }

    /* filter when in edge sensitive  */
    if (0x0 == chn->filterBypass) {
        *v &= ~BM_WAKEUP_CTRL_WKUP1_EDGE;
        /* TODO same other config need be realize */
    }
    else {
        /* do nothing */
    }

    /* enable wakeup 1 */
    if (PMU_WAKEUP_ENABLE == chn->enable) {
        *v &= ~BM_WAKEUP_CTRL_WKUP1_DISABLE;
    }
    else {
        *v |= BM_WAKEUP_CTRL_WKUP1_DISABLE;
    }
}

void sdrv_pmu_lld_wakeup_init(paddr_t base, const sdrv_pmu_wakeup_t *wakeup)
{
    uint32_t v = readl(base + WAKEUP_CTRL_OFF);

    /* set chn seperate */
    sdrv_pmu_lld_wakeup_0_init(&wakeup->Chn[0], &v);
    sdrv_pmu_lld_wakeup_1_init(&wakeup->Chn[1], &v);

    /* wakeup0/1 both active */
    if (0x1 == wakeup->wakeupAND) {
        v |= BM_WAKEUP_CTRL_WKUP01_AND_ENABLE;
    }

    /* latch wakeup in rtc mode */
    if (0x1 == wakeup->latchWakeup) {
        v |= BM_WAKEUP_CTRL_WAKEUP_IN_RTC_LATCH;
    }

    /* enable internal wakeup in rtc mode */
    if (0x1 == wakeup->internalWakeup) {
        v |= BM_WAKEUP_CTRL_INT_WAKEUP_EN;
    }

    writel(v, base + WAKEUP_CTRL_OFF);
}

void sdrv_pmu_lld_wakeup_deinit(paddr_t base)
{
    uint32_t v = readl(base + WAKEUP_CTRL_OFF);

    v |= BM_WAKEUP_CTRL_WKUP0_DISABLE;
    v |= BM_WAKEUP_CTRL_WKUP1_DISABLE;

    writel(v, base + WAKEUP_CTRL_OFF);
}

/*
 *  wakeup filter (FILTER_CNT+2)*(2^SAMPLE_INTERVAL)=(1+2)*(2^0)=3*0.03125=0.1ms
 *  32K = 0.03125ms
 */
void sdrv_pmu_lld_wakeup_filter(paddr_t base, uint8_t chn, uint8_t filterCnt,
                                uint8_t SampleInterval)
{
    uint32_t v = readl(base + WAKEUP_CTRL_OFF);

    if (0 == chn) {
        v &= ~FM_WAKEUP_CTRL_WKUP0_FILTER_CNT;
        v &= ~FM_WAKEUP_CTRL_WKUP0_SAMPLE_INTERVAL;
        v |= FV_WAKEUP_CTRL_WKUP0_FILTER_CNT(filterCnt);
        v |= FV_WAKEUP_CTRL_WKUP0_SAMPLE_INTERVAL(SampleInterval);
    }
    else {
        v &= ~FM_WAKEUP_CTRL_WKUP1_FILTER_CNT;
        v &= ~FM_WAKEUP_CTRL_WKUP1_SAMPLE_INTERVAL;
        v |= FV_WAKEUP_CTRL_WKUP1_FILTER_CNT(filterCnt);
        v |= FV_WAKEUP_CTRL_WKUP1_SAMPLE_INTERVAL(SampleInterval);
    }

    writel(v, base + WAKEUP_CTRL_OFF);
}

/*
 * enable BC button wakeup
 */
void sdrv_pmu_lld_bc_wakeup_enable(paddr_t base, sdrv_pmu_wakep_enable enable)
{
    uint32_t v = readl(base + WAKEUP_CTRL_OFF);

    if (PMU_WAKEUP_ENABLE == enable) {
        v |= BM_WAKEUP_CTRL_EXT_WAKEUP_EN;
    }
    else {
        v &= ~BM_WAKEUP_CTRL_EXT_WAKEUP_EN;
    }

    writel(v, base + WAKEUP_CTRL_OFF);
}

/*
 * 0:wakeup by button control
 * 1:rtc periodic interrupt
 * 2:wakeup button
 * 3:auto wakeup which is triggered by hardware power down request.
 */
uint32_t sdrv_pmu_lld_get_wakeup_reason(paddr_t base)
{
    uint32_t v = readl(base + PMU_DOWN_UP_STATUS_OFF);
    v &= 0xF;
    return v;
}

uint32_t sdrv_pmu_lld_get_powerdown_reason(paddr_t base)
{
    uint32_t v = readl(base + PMU_DOWN_UP_STATUS_OFF);
    v = (v >> 4);
    return v;
}

/******************************************************************************
 * button control
 *
 *****************************************************************************/
void sdrv_pmu_lld_bc_init(paddr_t base, sdrv_pmu_bc_ctrl_t *wakeup)
{
    uint32_t v = readl(base + BC_CTRL_SET_OFF);

    /* set sensitive level */
    if (PMU_BC_EDGE_SENSITIVE == wakeup->sensitive) {
        v |= BM_BC_CTRL_SET_BC_ON_EDGE;
        v |= BM_BC_CTRL_SET_BC_OFF_EDGE;
        /* TODO:filter can be realize if needed */
    }
    else {
        v &= ~BM_BC_CTRL_SET_BC_ON_EDGE;
        v &= ~BM_BC_CTRL_SET_BC_OFF_EDGE;
        /* TODO:max window can be realize if need */
    }

    /* set polarity */
    if (0x1 == wakeup->polarity) {
        /* ON:low active or falling edge OFF:high active or rising edge */
        v |= BM_BC_CTRL_SET_BC_ON_POL;
        v &= ~BM_BC_CTRL_SET_BC_OFF_POL;
    }
    else {
        /* ON:high active or rising edge OFF:low active or failing edge */
        v &= ~BM_BC_CTRL_SET_BC_ON_POL;
        v |= BM_BC_CTRL_SET_BC_OFF_POL;
    }

    /* enable button control */
    v &= ~BM_BC_CTRL_SET_BC_DISABLE;

    writel(v, base + BC_CTRL_SET_OFF);
}

void sdrv_pmu_lld_bc_deinit(paddr_t base)
{
    uint32_t v = readl(base + BC_CTRL_SET_OFF);

    v |= BM_BC_CTRL_SET_BC_DISABLE;

    writel(v, base + BC_CTRL_SET_OFF);
}

void sdrv_pmu_lld_bc_enbale(paddr_t base)
{
    uint32_t v = readl(base + BC_CTRL_SET_OFF);

    v &= ~BM_BC_CTRL_SET_BC_DISABLE;

    writel(v, base + BC_CTRL_SET_OFF);
}

/* delay time = BC_ON/OFF_VLD_MIN/MAX*(2^BC_CNT_UNIT)
 *  default value BC_CNT_UNIT = 5
 */
void sdrv_pmu_lld_bc_max_off_window(paddr_t base, uint8_t time)
{
    uint32_t v = readl(base + BC_VLD_WINDOW_OFF);

    v &= ~FM_BC_VLD_WINDOW_BC_OFF_VLD_MAX;
    v |= FV_BC_VLD_WINDOW_BC_OFF_VLD_MAX(time);

    writel(v, base + BC_VLD_WINDOW_OFF);
}

void sdrv_pmu_lld_bc_min_off_window(paddr_t base, uint8_t time)
{
    uint32_t v = readl(base + BC_VLD_WINDOW_OFF);

    v &= ~FM_BC_VLD_WINDOW_BC_OFF_VLD_MIN;
    v |= FV_BC_VLD_WINDOW_BC_OFF_VLD_MIN(time);

    writel(v, base + BC_VLD_WINDOW_OFF);
}

void sdrv_pmu_lld_bc_max_on_window(paddr_t base, uint8_t time)
{
    uint32_t v = readl(base + BC_VLD_WINDOW_OFF);

    v &= ~FM_BC_VLD_WINDOW_BC_ON_VLD_MAX;
    v |= FV_BC_VLD_WINDOW_BC_ON_VLD_MAX(time);

    writel(v, base + BC_VLD_WINDOW_OFF);
}

void sdrv_pmu_lld_bc_min_on_window(paddr_t base, uint8_t time)
{
    uint32_t v = readl(base + BC_VLD_WINDOW_OFF);

    v &= ~FM_BC_VLD_WINDOW_BC_ON_VLD_MIN;
    v |= FV_BC_VLD_WINDOW_BC_ON_VLD_MIN(time);

    writel(v, base + BC_VLD_WINDOW_OFF);
}

/* disable bc max off window check */
void sdrv_pmu_lld_bc_max_off_window_enable(paddr_t base, uint8_t status)
{
    uint32_t v = readl(base + BC_CTRL_SET_OFF);

    if (status == 0x0) {
        /* enable */
        v &= ~BM_BC_CTRL_SET_BC_OFF_VLD_MAX_DISABLE;
    }
    else {
        /* disable */
        v |= BM_BC_CTRL_SET_BC_OFF_VLD_MAX_DISABLE;
    }

    writel(v, base + BC_CTRL_SET_OFF);
}

/* disable bc max on window check */
void sdrv_pmu_lld_bc_max_on_window_enable(paddr_t base, uint8_t status)
{
    uint32_t v = readl(base + BC_CTRL_SET_OFF);

    if (status == 0x0) {
        /* enable */
        v &= ~BM_BC_CTRL_SET_BC_ON_VLD_MAX_DISABLE;
    }
    else {
        /* disable */
        v |= BM_BC_CTRL_SET_BC_ON_VLD_MAX_DISABLE;
    }

    writel(v, base + BC_CTRL_SET_OFF);
}

void sdrv_pmu_lld_pwrdown_request(paddr_t base)
{
    uint32_t v = readl(base + BC_CTRL_SET_OFF);

    v |= BM_BC_CTRL_SET_PWR_DWN_REQ_SOFT;

    writel(v, base + BC_CTRL_SET_OFF);
}

/******************************************************************************
 * pmu power on monitor
 *
 *****************************************************************************/
void sdrv_pmu_lld_pwr_on_wdg_enable(paddr_t base)
{
    uint32_t v = readl(base + PGM_CTRL_OFF);

    v |= FV_PGM_CTRL_PWR_ON_WDT_EN(0xF);

    writel(v, base + WAKEUP_CTRL_OFF);
}

uint32_t sdrv_pmu_lld_get_int_status(paddr_t base)
{
    uint32_t v = readl(base + PMU_INT_OFF);
    uint32_t ret = v & 0x1FFF;

    return ret;
}

/* pwr on ready status[3:0]*/
uint32_t sdrv_pmu_lld_get_pwr_rdy_status(paddr_t base)
{
    uint32_t v = readl(base + PMU_STATE_OFF);
    v = (v >> 4) & 0xF;

    return v;
}
/*
 * enable pwr_on to pwr_rdy wdg monitior, the default value is 1s
 * Timeout = pwr_on_wdg_cnt*(2^wdg_div)
 * 960ms = 1/32K*0xF*2^11
 */
void sdrv_pmu_lld_pwr_on_wdg_en(paddr_t base)
{
    uint32_t v = readl(base + PGM_CTRL_OFF);

    v |= FV_PGM_CTRL_PWR_ON_WDT_EN(0xF);

    writel(v, base + PGM_CTRL_OFF);
}

/******************************************************************************
 * pmu power good ctrl and monitor
 *
 *****************************************************************************/
void sdrv_pmu_lld_sf_por_pg_enable(paddr_t base, sdrv_pmu_pg_ctrl_t enable)
{
    uint32_t v = readl(base + PG_LP_MODE_CTRL_OFF);

    if (enable == PMU_PG_ENABLE) {
        v &= ~BM_PG_LP_MODE_CTRL_SF_HV_PG_DISABLE;
        v &= ~BM_PG_LP_MODE_CTRL_SF_LV_PG_DISABLE;
    }
    else {
        v |= BM_PG_LP_MODE_CTRL_SF_HV_PG_DISABLE;
        v |= BM_PG_LP_MODE_CTRL_SF_LV_PG_DISABLE;
    }

    writel(v, base + PG_LP_MODE_CTRL_OFF);
}

void sdrv_pmu_lld_ap_por_pg_enable(paddr_t base, sdrv_pmu_pg_ctrl_t enable)
{
    uint32_t v = readl(base + PG_LP_MODE_CTRL_OFF);

    if (enable == PMU_PG_ENABLE) {
        v &= ~BM_PG_LP_MODE_CTRL_AP_HV_PG_DISABLE;
        v &= ~BM_PG_LP_MODE_CTRL_AP_LV_PG_DISABLE;
    }
    else {
        v |= BM_PG_LP_MODE_CTRL_AP_HV_PG_DISABLE;
        v |= BM_PG_LP_MODE_CTRL_AP_LV_PG_DISABLE;
    }

    writel(v, base + PG_LP_MODE_CTRL_OFF);
}

/* enum pmu_pg_status */
uint32_t sdrv_pmu_lld_get_pg_status(paddr_t base)
{
    uint32_t v = readl(base + PMU_STATE_OFF);
    v = v >> 8;

    return v;
}
