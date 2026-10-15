#ifdef CONFIG_SDRV_SMC

#include <types.h>
#include <param.h>
#include "smc_cfg.h"
#include "clock_cfg.h"
#include "rstgen_cfg.h"
#include "irq_num.h"

static struct sdrv_smc_clock enable_clks[] = {
    {NULL, 0, 0},
};

static struct sdrv_smc_reset reset_sigs[] = {
    {NULL, 0, 0},
};

static struct sdrv_smc_wakeup_irq wakeup_irqs[] = {
    {SMC_INVALID_CORE_ID, 0},
};

struct sdrv_smc_ckgen_config g_smc_ckgen_cfg = {
    .enable_clk     = enable_clks,
    .enable_clk_num = ARRAY_SIZE(enable_clks) - 1,

    .pll1 = {
        .sleep_mode_disable = false,
        .hib_mode_disable   = false,
        .sleep_mode_pd      = false,
        .hib_mode_pd        = false,
    },

    .pll2 = {
        .sleep_mode_disable = false,
        .hib_mode_disable   = false,
        .sleep_mode_pd      = false,
        .hib_mode_pd        = false,
    },

    .pll3 = {
        .sleep_mode_disable = false,
        .hib_mode_disable   = false,
        .sleep_mode_pd      = false,
        .hib_mode_pd        = false,
    },

    .pll4 = {
        .sleep_mode_disable = false,
        .hib_mode_disable   = false,
        .sleep_mode_pd      = false,
        .hib_mode_pd        = false,
    },

    .pll5 = {
        .sleep_mode_disable = false,
        .hib_mode_disable   = false,
        .sleep_mode_pd      = false,
        .hib_mode_pd        = false,
    },

    .lvds = {
        .sleep_mode_disable = false,
        .hib_mode_disable   = false,
        .sleep_mode_pd      = false,
        .hib_mode_pd        = false,
    },

    /* XTAL */
    .saf = {
        .sleep_mode_disable = false,
        .hib_mode_disable   = false,
    },

    /* XTAL */
    .ap = {
        .sleep_mode_disable = false,
        .hib_mode_disable   = false,
    },
};

struct sdrv_smc_rstgen_config g_smc_rstgen_cfg = {
    .smc_reset     = reset_sigs,
    .smc_reset_num = ARRAY_SIZE(reset_sigs) - 1,
};

struct sdrv_smc_pmu_config g_smc_pmu_cfg = {
    .sleep = {
        .pwr_ctrl0 = true,
        .pwr_ctrl1 = true,
        .pwr_ctrl2 = true,
        .pwr_ctrl3 = true,
        .pwr_on0   = true,
        .pwr_on1   = true,
        .pwr_on2   = true,
        .pwr_on3   = true,
        .saf_power_on = true,
        .ap_power_on  = true,
        .pwron0_ctrl_target = PWRON_CTRL_TARGET_SF,
        .pwron1_ctrl_target = PWRON_CTRL_TARGET_SF,
        .pwron2_ctrl_target = PWRON_CTRL_TARGET_SF,
        .pwron3_ctrl_target = PWRON_CTRL_TARGET_SF,
    },

    .hib = {
        .pwr_ctrl0 = true,
        .pwr_ctrl1 = false,
        .pwr_ctrl2 = true,
        .pwr_ctrl3 = false,
        .pwr_on0   = true,
        .pwr_on1   = true,
        .pwr_on2   = false,
        .pwr_on3   = false,
        .saf_power_on = true,
        .ap_power_on  = true,
        .pwron0_ctrl_target = PWRON_CTRL_TARGET_SF,
        .pwron1_ctrl_target = PWRON_CTRL_TARGET_SF,
        .pwron2_ctrl_target = PWRON_CTRL_TARGET_SF,
        .pwron3_ctrl_target = PWRON_CTRL_TARGET_SF,
    },
};

struct sdrv_smc_config g_smc_cfg = {
    /* Safety Domain */
    .saf_primary_core = 0,

    /* power switch */
    .sf_mix = {
        .hib_pd_enable   = false,
        .sleep_pd_enable = false,
    },

    .sp_mix = {
        .hib_pd_enable   = false,
        .sleep_pd_enable = false,
    },

    .sx_mix = {
        .hib_pd_enable   = false,
        .sleep_pd_enable = false,
    },

    .gama1_pwr = {
        .hib_pd_enable   = false,
        .sleep_pd_enable = false,
    },

    /* ram */
    .cr5_sf = {
        .hib_ram_lp_setting   = RAM_LP_RET1N | RAM_LP_RET2N,
        .sleep_ram_lp_setting = RAM_LP_RET1N | RAM_LP_RET2N,
        .hib_ram_pd_enable    = false,
        .sleep_ram_pd_enable  = false,
    },

    .cr5_sp = {
        .hib_ram_lp_setting   = RAM_LP_RET1N | RAM_LP_RET2N,
        .sleep_ram_lp_setting = RAM_LP_RET1N | RAM_LP_RET2N,
        .hib_ram_pd_enable    = false,
        .sleep_ram_pd_enable  = false,
    },

    .cr5_sx = {
        .hib_ram_lp_setting   = RAM_LP_RET1N | RAM_LP_RET2N,
        .sleep_ram_lp_setting = RAM_LP_RET1N | RAM_LP_RET2N,
        .hib_ram_pd_enable    = false,
        .sleep_ram_pd_enable  = false,
    },

    .gama1_ram = {
        .hib_ram_lp_setting   = RAM_LP_RET1N | RAM_LP_RET2N,
        .sleep_ram_lp_setting = RAM_LP_RET1N | RAM_LP_RET2N,
        .hib_ram_pd_enable    = false,
        .sleep_ram_pd_enable  = false,
    },

    .saf_misc = {
        .hib_ram_lp_setting   = RAM_LP_RET1N | RAM_LP_RET2N,
        .sleep_ram_lp_setting = RAM_LP_RET1N | RAM_LP_RET2N,
        .hib_ram_pd_enable    = false,
        .sleep_ram_pd_enable  = false,
    },

    /* AP Domain */
    .ap_primary_core  = 0,
    .ap_lp_align_saf  = false,
    .ap_wk_align_saf  = false,

    /* power switch */
    .ap_disp_ss = {
        .hib_pd_enable   = false,
        .sleep_pd_enable = false,
    },

    /* ram */
    .ap_ram = {
        .hib_ram_lp_setting   = RAM_LP_RET1N | RAM_LP_RET2N,
        .sleep_ram_lp_setting = RAM_LP_RET1N | RAM_LP_RET2N,
        .hib_ram_pd_enable    = false,
        .sleep_ram_pd_enable  = false,
    },

    .disp_ram = {
        .hib_ram_lp_setting   = RAM_LP_RET1N | RAM_LP_RET2N,
        .sleep_ram_lp_setting = RAM_LP_RET1N | RAM_LP_RET2N,
        .hib_ram_pd_enable    = false,
        .sleep_ram_pd_enable  = false,
    },

    /* SOC */
    .ap_off            = false,
    .hib_disable_rc24m = false,

    .wk_irq     = wakeup_irqs,
    .wk_irq_num = ARRAY_SIZE(wakeup_irqs) - 1,
};

#endif /* CONFIG_SDRV_SMC */
