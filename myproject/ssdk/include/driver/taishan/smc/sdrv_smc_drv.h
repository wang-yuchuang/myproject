/**
 * @file sdrv_smc_drv.h
 * @brief Taishan SMC head source.
 *
 * @copyright Copyright (c) 2022  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef SDRV_SMC_DRV_H_
#define SDRV_SMC_DRV_H_

#include <types.h>
#include <driver/taishan/clk/taishan_ckgen_drv.h>
#include <driver/taishan/reset/taishan_reset_drv.h>

#define SMC_ASYNC_INT_USB            (236 + 16)
#define SMC_ASYNC_INT_IOC            (237 + 16)
#define SMC_ASYNC_INT_VIC1_SF_GPIO   (238 + 16)
#define SMC_ASYNC_INT_VIC1_AP_GPIO   (239 + 16)
#define SMC_ASYNC_INT_VIC2A_SF_GPIO  (240 + 16)
#define SMC_ASYNC_INT_VIC2A_AP_GPIO  (241 + 16)
#define SMC_ASYNC_INT_VIC2B_SF_GPIO  (242 + 16)
#define SMC_ASYNC_INT_VIC2B_AP_GPIO  (243 + 16)
#define SMC_ASYNC_INT_VIC3A_SF_GPIO  (244 + 16)
#define SMC_ASYNC_INT_VIC3A_AP_GPIO  (245 + 16)
#define SMC_ASYNC_INT_VIC3B_SF_GPIO  (246 + 16)
#define SMC_ASYNC_INT_VIC3B_AP_GPIO  (247 + 16)

#define SMC_CORE_SF  0
#define SMC_CORE_SP0 1
#define SMC_CORE_SP1 2
#define SMC_CORE_SX0 3
#define SMC_CORE_SX1 4
#define SMC_CORE_MAX 5

#define SMC_HK_CKGEN  0
#define SMC_HK_RSTGEN 1
#define SMC_HK_PMU    2

#define SMC_HK_SAF    0
#define SMC_HK_AP     1

#define SMC_HK_RUN    0
#define SMC_HK_SLP    1
#define SMC_HK_HIB    2
#define SMC_HK_RTC    3

#define RAM_LP_PG_EN  0x4u
#define RAM_LP_RET1N  0x2u
#define RAM_LP_RET2N  0x1u

#define RAM_MODE_CHIP_ENABLE          (RAM_LP_RET1N | RAM_LP_RET2N)
#define RAM_MODE_SELECTIVE_PRECHARGE  (RAM_LP_RET2N)
#define RAM_MODE_RETENTION_1          (RAM_LP_PG_EN | RAM_LP_RET2N)
#define RAM_MODE_RETENTION_2          (RAM_LP_PG_EN | RAM_LP_RET1N)
#define RAM_MODE_POWER_DOWN           (RAM_LP_PG_EN | RAM_LP_RET1N | RAM_LP_RET2N)

#define SMC_INVALID_CORE_ID  0xffffffffu

typedef enum {
    SMC_SAF_DOMAIN,
    SMC_AP_DOMAIN,
} smc_domain;

typedef enum {
    SMC_SAF_CR5_SF = 0,
    SMC_SAF_CR5_SP,
    SMC_SAF_CR5_SX,
    SMC_SAF_CR5_GAMA1,
    SMC_SAF_MISC,
} smc_saf_lp_ctrl;

typedef enum {
    SMC_AP_SS = 0,
    SMC_DISP_SS,
} smc_ap_lp_ctrl;

struct sdrv_smc_lp_ctrl {
    bool hib_pd_enable;
    bool sleep_pd_enable;
};

struct sdrv_smc_ram_lp_ctrl {
    uint32_t hib_ram_lp_setting;
    uint32_t sleep_ram_lp_setting;

    bool hib_ram_pd_enable;
    bool sleep_ram_pd_enable;
};

/* Config for wakeup irq, ckgen, rstgen, pmu */
struct sdrv_smc_wakeup_irq {
    uint32_t core;
    uint32_t irq;
};

struct sdrv_smc_clock {
    struct taishan_cg_node *peripheral;
    bool hib_enable;
    bool sleep_enable;
};

struct sdrv_smc_pll {
    bool sleep_mode_disable;
    bool hib_mode_disable;

    bool sleep_mode_pd;
    bool hib_mode_pd;
};

struct sdrv_smc_xtal {
    bool sleep_mode_disable;
    bool hib_mode_disable;
};

struct sdrv_smc_ckgen_config {
    /* Default all clock disable*/
    struct sdrv_smc_clock *enable_clk;
    uint32_t enable_clk_num;

    struct sdrv_smc_pll pll1;
    struct sdrv_smc_pll pll2;
    struct sdrv_smc_pll pll3;
    struct sdrv_smc_pll pll4;
    struct sdrv_smc_pll pll5;
    struct sdrv_smc_pll lvds;

    struct sdrv_smc_xtal saf;
    struct sdrv_smc_xtal ap;
};

struct sdrv_smc_reset {
    struct reset_sig *sig;
    bool sleep_mode_reset;
    bool hib_mode_reset;
};

struct sdrv_smc_rstgen_config {
    struct sdrv_smc_reset *smc_reset;
    uint32_t smc_reset_num;
};

/*
PMU default power state configuration

STATE     CONFIG        PWR_CTRL0   PWR_CTRL1   PWR_CTRL2   PWR_CTRL3   PWR_ON0   PWR_ON1   PWR_ON2   PWR_ON3
--------------------------------------------------------------------------------------------------------------
        ctrl_target        n/a         n/a         n/a         n/a        SF         SF        SF        SF
RUN      on / off          on          on          on          on         on         on        on        on
       auto / manual       A           A           A           M          A          A         A         A
--------------------------------------------------------------------------------------------------------------
        ctrl_target        n/a         n/a         n/a         n/a        SF         SF        SF        SF
SLEEP    on / off          on          on          on          on         on         on        on        on
       auto / manual       A           A           A           M          A          A         A         A
--------------------------------------------------------------------------------------------------------------
        ctrl_target        n/a         n/a         n/a         n/a        SF         SF        SF        SF
HIB      on / off          on          off         on          off        on         on        off       off
       auto / manual       A           A           A           M          A          A         A         A

*/
struct sdrv_smc_pmu {
    bool pwr_ctrl0;
    bool pwr_ctrl1;
    bool pwr_ctrl2;
    bool pwr_ctrl3;

    /* control SAF/AP domain global reset */
    bool saf_power_on;
    bool ap_power_on;

    bool pwr_on0;
    bool pwr_on1;
    bool pwr_on2;
    bool pwr_on3;

#define PWRON_CTRL_TARGET_SF     0x0u
#define PWRON_CTRL_TARGET_AP     0x1u
#define PWRON_CTRL_TARGET_OTHER  0x2u
    uint32_t pwron0_ctrl_target;
    uint32_t pwron1_ctrl_target;
    uint32_t pwron2_ctrl_target;
    uint32_t pwron3_ctrl_target;
};

struct sdrv_smc_pmu_config {
    struct sdrv_smc_pmu sleep;
    struct sdrv_smc_pmu hib;
};

struct sdrv_smc_config {
    /* Safety domain */
    uint32_t saf_primary_core;

    struct sdrv_smc_lp_ctrl sf_mix;
    struct sdrv_smc_lp_ctrl sp_mix;
    struct sdrv_smc_lp_ctrl sx_mix;
    struct sdrv_smc_lp_ctrl gama1_pwr;

    struct sdrv_smc_ram_lp_ctrl cr5_sf;
    struct sdrv_smc_ram_lp_ctrl cr5_sp;
    struct sdrv_smc_ram_lp_ctrl cr5_sx;
    struct sdrv_smc_ram_lp_ctrl gama1_ram;
    struct sdrv_smc_ram_lp_ctrl saf_misc;

    /* AP domain */
    uint32_t ap_primary_core;

    bool ap_lp_align_saf;
    bool ap_wk_align_saf;

    struct sdrv_smc_lp_ctrl ap_disp_ss;
    struct sdrv_smc_ram_lp_ctrl ap_ram;
    struct sdrv_smc_ram_lp_ctrl disp_ram;

    /* SOC */
    bool ap_off; /* AP domain power off, SMC will only handshake with SAF */
    bool hib_disable_rc24m;

    /* Wakeup Irq */
    struct sdrv_smc_wakeup_irq *wk_irq;
    uint32_t wk_irq_num;
};

/**
 * @brief software trigger SWM change
 *
 * @param module see macro SMC_HK_**
 * @param domain see macro SMC_HK_**
 * @param lp_mode see macro SMC_HK_**
 */
void sdrv_smc_sw_trigger_handshake(uint32_t module, uint32_t domain, uint32_t lp_mode);

void sdrv_smc_sw_power_switch_gate(smc_domain domain, uint32_t lp_ctrl, bool gate);

void sdrv_smc_sw_ram_lp_ctrl(smc_domain domain, uint32_t lp_ctrl, bool pd, uint32_t ram_mode);

void sdrv_smc_wait_soc_swm_run(void);

void sdrv_smc_wait_ap_sys_ready(void);

void sdrv_smc_clear_all_irq_mon(void);

void sdrv_smc_lp_request(uint32_t saf_mode, uint32_t ap_mode);

void sdrv_smc_lp_disable(void);

void sdrv_smc_ckgen_config(struct sdrv_smc_ckgen_config *ckgen_config);

void sdrv_smc_rstgen_config(struct sdrv_smc_rstgen_config *rstgen_config);

void sdrv_smc_pmu_config(struct sdrv_smc_pmu_config *pmu_config);

void sdrv_smc_init(struct sdrv_smc_config *smc_config);

#endif /* SDRV_SMC_DRV_H_ */
