#ifdef CONFIG_SDRV_SMC
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cmsis_os2.h>
#include <CLI_type.h>
#include <debug.h>
#include <board.h>
#include <driver/taishan/smc/sdrv_smc_drv.h>
#include <driver/pm.h>
#include <driver/taishan/power/lp_utils.h>
#include <driver/gpio.h>
#include <driver/taishan/pinctrl/taishan_pinctrl.h>
#include <driver/irq.h>
#include <types.h>
#include <param.h>
#include <reg.h>
#include "clock_cfg.h"
#include "rstgen_cfg.h"
#include "irq_num.h"
#include "pinmux_cfg.h"

#ifdef CONFIG_SBC
#include "../../examples/taishan/drivers/sbc/sbc_sys.h"
#endif

extern gpio_dev_t g_gpio_safety;
extern gpio_dev_t g_gpio_ap;
#define GPIO_INSTANCE(pin)  \
    (((pin) < TAISHAN_SAFETY_PIN_NUM) ? &g_gpio_safety : &g_gpio_ap)

extern pinctrl_t g_pinctrl_safety;
extern pinctrl_t g_pinctrl_ap;
#define PINCTRL_INSTANCE(pin)  \
    (((pin) < TAISHAN_SAFETY_PIN_NUM) ? &g_pinctrl_safety : &g_pinctrl_ap)

static void lp_test_bus_core_clk_switch_rc24m(void);
static void lp_test_bus_core_clk_switch_config(void);

/* E3-BGA324-EVB wakeup source: uart6 */
static struct sdrv_smc_clock uart6_clks[] = {
    /* 0 */
    {
        .peripheral = &g_ckgen_gating_uart6_pclk,
        .hib_enable = true,
        .sleep_enable = true,
    },

    /* 1 */
    {
        .peripheral = &g_ckgen_gating_uart6_sclk,
        .hib_enable = true,
        .sleep_enable = true,
    },
};

static struct sdrv_smc_wakeup_irq uart6_irqs[] = {
    {
        .core = SMC_CORE_SF,
        .irq = UART6_INTR_NUM,
    },
};

static struct sdrv_smc_wakeup_irq uart6_rtc1_irqs[] = {
    {
        .core = SMC_CORE_SF,
        .irq = UART6_INTR_NUM,
    },

    {
        .core = SMC_CORE_SF,
        .irq = RTC1_RTC_WAKEUP_INTR_NUM,
    },
};

static struct sdrv_smc_wakeup_irq uart7_rtc1_irqs[] = {
    {
        .core = SMC_CORE_SF,
        .irq = UART7_INTR_NUM,
    },

    {
        .core = SMC_CORE_SF,
        .irq = RTC1_RTC_WAKEUP_INTR_NUM,
    },
};

static struct sdrv_smc_wakeup_irq gpio_irqs[] = {
    {
        /* gpio_a0 all in saf domain */
        .core = SMC_CORE_SF,
        .irq = SMC_ASYNC_INT_VIC1_SF_GPIO,
    },
};

/* E3-BGA324-Gateway wakeup source: uart7 */
static struct sdrv_smc_clock uart7_clks[] = {
    /* 0 */
    {
        .peripheral = &g_ckgen_gating_uart7_pclk,
        .hib_enable = true,
        .sleep_enable = true,
    },

    /* 1 */
    {
        .peripheral = &g_ckgen_gating_uart7_sclk,
        .hib_enable = true,
        .sleep_enable = true,
    },

    /* !!!! keep sbc clock !!!! */
#ifdef CONFIG_SBC
    {
        .peripheral = &g_ckgen_gating_spi7_pclk,
        .hib_enable = true,
        .sleep_enable = true,
    },

    {
        .peripheral = &g_ckgen_gating_spi7_spi_per_clk,
        .hib_enable = true,
        .sleep_enable = true,
    },
#endif
};

#ifdef CONFIG_SBC
static struct sdrv_smc_clock sbc_clk[] = {
    {
        .peripheral = &g_ckgen_gating_spi7_pclk,
        .hib_enable = true,
        .sleep_enable = true,
    },

    {
        .peripheral = &g_ckgen_gating_spi7_spi_per_clk,
        .hib_enable = true,
        .sleep_enable = true,
    },
};
#endif

static struct sdrv_smc_wakeup_irq uart7_irqs[] = {
    {
        .core = SMC_CORE_SF,
        .irq = UART7_INTR_NUM,
    },
};

/* E3-BGA324-Disp wakeup source: uart4 */
static struct sdrv_smc_clock uart4_clks[] = {
    /* 0 */
    {
        .peripheral = &g_ckgen_gating_uart4_pclk,
        .hib_enable = true,
        .sleep_enable = true,
    },

    /* 1 */
    {
        .peripheral = &g_ckgen_gating_uart4_sclk,
        .hib_enable = true,
        .sleep_enable = true,
    },
};

static struct sdrv_smc_wakeup_irq uart4_irqs[] = {
    {
        .core = SMC_CORE_SF,
        .irq = UART4_INTR_NUM,
    },
};

static struct sdrv_smc_wakeup_irq uart4_rtc1_irqs[] = {
    {
        .core = SMC_CORE_SF,
        .irq = UART4_INTR_NUM,
    },

    {
        .core = SMC_CORE_SF,
        .irq = RTC1_RTC_WAKEUP_INTR_NUM,
    },
};

static struct sdrv_smc_wakeup_irq rtc1_irq[] = {
    {
        .core = SMC_CORE_SF,
        .irq = RTC1_RTC_WAKEUP_INTR_NUM,
    },
};

static struct sdrv_smc_clock uart_clks_176_144[] = {
    {
        .peripheral = &g_ckgen_gating_uart7_pclk,
        .hib_enable = true,
        .sleep_enable = true,
    },

    {
        .peripheral = &g_ckgen_gating_uart7_sclk,
        .hib_enable = true,
        .sleep_enable = true,
    },
};

static struct sdrv_smc_wakeup_irq uart_irq_176_144[] = {
    {
        .core = SMC_CORE_SF,
        .irq = UART7_INTR_NUM,
    },
};

static struct sdrv_smc_clock canfd_clks_176_144[] = {
    {
        .peripheral = &g_ckgen_gating_canfd3_pclk,
        .hib_enable = true,
        .sleep_enable = true,
    },

    {
        .peripheral = &g_ckgen_gating_canfd3_ipg_clk,
        .hib_enable = true,
        .sleep_enable = true,
    },

    {
        .peripheral = &g_ckgen_gating_canfd3_clk24m,
        .hib_enable = true,
        .sleep_enable = true,
    },

    {
        .peripheral = &g_ckgen_gating_canfd4_pclk,
        .hib_enable = true,
        .sleep_enable = true,
    },

    {
        .peripheral = &g_ckgen_gating_canfd4_ipg_clk,
        .hib_enable = true,
        .sleep_enable = true,
    },

    {
        .peripheral = &g_ckgen_gating_canfd4_clk24m,
        .hib_enable = true,
        .sleep_enable = true,
    },
};

static struct sdrv_smc_wakeup_irq canfd_irq_176_144[] = {
    {
        .core = SMC_CORE_SF,
        .irq = CANFD3_CANFD_INTR_NUM,
    },

    {
        .core = SMC_CORE_SF,
        .irq = CANFD4_CANFD_INTR_NUM,
    },
};

#if !(defined(CONFIG_BOARD_E3_176_REF) || defined(CONFIG_BOARD_E3_144_REF))
static struct sdrv_smc_reset reset_sigs_case0[] = {
    /* vic & mailbox not reset in sleep */

    /* 0 */
    {
        .sig = &rstsig_vic1,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },

    /* 1 */
    {
        .sig = &rstsig_vic2_porta,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },

    /* 2 */
    {
        .sig = &rstsig_vic2_portb,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },

    /* 3 */
    {
        .sig = &rstsig_vic3_porta,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },

    /* 4 */
    {
        .sig = &rstsig_vic3_portb,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },

    /* 5 */
    {
        .sig = &rstsig_mb,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },

    /* core reset in hib */

    /* 6 */
    {
        .sig = &rstsig_cr5_saf,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },

    /* 7 */
    {
        .sig = &rstsig_cr5_sp0,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },

    /* 8 */
    {
        .sig = &rstsig_cr5_sp1,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },

    /* 9 */
    {
        .sig = &rstsig_cr5_sx0,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },

    /* 10 */
    {
        .sig = &rstsig_cr5_sx1,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },

    /* 11 */
    {
        .sig = &rstsig_saf_mission6,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },

    /* 12 */
    {
        .sig = &rstsig_ap_mission1,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },

    /* 13 */
    {
        .sig = &rstsig_saf_debug,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },

    /* 14 */
    {
        .sig = &rstsig_ap_debug,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },

    /* for display board */
#ifdef CONFIG_SDRAMC
    {
        .sig = &rstsig_sdramc,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },
#endif

    /* for gateway board */
    /* !!!! XIP mode XSPI can't reset !!!! */
#ifdef CONFIG_XIP_MODE
    {
        .sig = &rstsig_xspi1a,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },

    {
        .sig = &rstsig_xspi1b,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },
#endif
};
#else /* CONFIG_BOARD_E3_176_REF
         CONFIG_BOARD_E3_144_REF */
static struct sdrv_smc_reset reset_sigs_case0[] = {
    /* 0 */
    {
        .sig = &rstsig_vic1,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },

    /* 1 */
    {
        .sig = &rstsig_canfd3_4,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },

    /* 2 */
    {
        .sig = &rstsig_cr5_saf,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },

    /* 3 */
    {
        .sig = &rstsig_saf_mission6,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },

    /* 4 */
    {
        .sig = &rstsig_ap_mission1,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },

    /* 5 */
    {
        .sig = &rstsig_saf_debug,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },

    /* 6 */
    {
        .sig = &rstsig_ap_debug,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },

    /* !!!! XIP mode XSPI can't reset !!!! */
#ifdef CONFIG_XIP_MODE
    {
        .sig = &rstsig_xspi1a,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },

    {
        .sig = &rstsig_xspi1b,
        .sleep_mode_reset = false,
        .hib_mode_reset = false,
    },
#endif
};
#endif

/*-----Common config for all test case, different case modify fields-----*/
static struct sdrv_smc_ckgen_config g_smc_ckgen_test_cfg = {
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

static struct sdrv_smc_rstgen_config g_smc_rstgen_test_cfg = {
    .smc_reset     = NULL,
    .smc_reset_num = 0,
};

static struct sdrv_smc_pmu_config g_smc_pmu_test_cfg = {
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
        .pwr_ctrl1 = true,
        .pwr_ctrl2 = true,
        .pwr_ctrl3 = false,
        .pwr_on0   = true,
        .pwr_on1   = true,
        .pwr_on2   = true,
        .pwr_on3   = false,
        .saf_power_on = true,
        .ap_power_on  = true,
        .pwron0_ctrl_target = PWRON_CTRL_TARGET_SF,
        .pwron1_ctrl_target = PWRON_CTRL_TARGET_SF,
        .pwron2_ctrl_target = PWRON_CTRL_TARGET_SF,
        .pwron3_ctrl_target = PWRON_CTRL_TARGET_SF,
    },
};

static struct sdrv_smc_config g_smc_test_cfg = {
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
};

static struct pm_callback_s lp_test_cb = {
    .cb_arg = NULL,
    .suspend = NULL,
    .resume = NULL,
};

#ifdef CONFIG_SBC
static int lp_test_sbc_suspend(struct pm_callback_s *cb, enum pm_state_e state)
{
    switch (state) {
        case PM_LIGHT_SLEEP:
            SBC_Mode_Stop_Without_WTD();
        break;

        case PM_DEEP_SLEEP:
            SBC_Mode_Stop_Without_WTD();
        break;

        case PM_STANDBY:
        case PM_SHUTDOWN:
        break;

        default:
        break;
    }

    return 0;
}

static void lp_test_sbc_resume(struct pm_callback_s *cb, enum pm_state_e state)
{
    switch (state) {
        case PM_LIGHT_SLEEP:
            // SBC_Set_Mode_Normal();
        break;

        case PM_DEEP_SLEEP:
            // SBC_Set_Mode_Normal();
        break;

        case PM_STANDBY:
        case PM_SHUTDOWN:
        break;

        default:
        break;
    }
}

static struct pm_callback_s lp_test_sbc_cb = {
    .suspend = lp_test_sbc_suspend,
    .resume = lp_test_sbc_resume,
};
#endif

static int lp_test_cb_disable_all_irq(struct pm_callback_s *cb, enum pm_state_e state)
{
    irq_mask(0);

    for (uint32_t i = 0; i < IRQ_MAX_INTR_NUM; i++) {
        irq_disable(i);
    }

    return 0;
}

static int lp_test_bus_core_uart6_clock_rc24m(struct pm_callback_s *cb, enum pm_state_e state)
{
    switch (state) {
        case PM_LIGHT_SLEEP:
            lp_test_bus_core_clk_switch_rc24m();
            lp_ckgen_node_switch_rc24m(&g_ckgen_ip_uart_sf_1_to_8);
        break;

        case PM_DEEP_SLEEP:
            lp_test_bus_core_clk_switch_rc24m();
            lp_ckgen_node_switch_rc24m(&g_ckgen_ip_uart_sf_1_to_8);
        break;

        case PM_STANDBY:
        case PM_SHUTDOWN:
        break;

        default:
        break;
    }

    return 0;
}

static void lp_test_bus_core_uart6_clock_resume(struct pm_callback_s *cb, enum pm_state_e state)
{
    switch (state) {
        case PM_LIGHT_SLEEP:
            lp_test_bus_core_clk_switch_config();
            lp_ckgen_node_switch_config(&g_ckgen_ip_uart_sf_1_to_8);
        break;

        case PM_DEEP_SLEEP:
            lp_test_bus_core_clk_switch_config();
            lp_ckgen_node_switch_config(&g_ckgen_ip_uart_sf_1_to_8);
        break;

        case PM_STANDBY:
        case PM_SHUTDOWN:
        break;

        default:
        break;
    }
}

static int lp_test_config_rtc_wakeup_timer(struct pm_callback_s *cb, enum pm_state_e state)
{
    switch (state) {
        case PM_LIGHT_SLEEP:
            lp_test_bus_core_clk_switch_rc24m();
            lp_ckgen_node_switch_rc24m(&g_ckgen_ip_uart_sf_1_to_8);
        break;

        case PM_DEEP_SLEEP:
            lp_test_bus_core_clk_switch_rc24m();
            lp_ckgen_node_switch_rc24m(&g_ckgen_ip_uart_sf_1_to_8);
            lp_rtc_wakeup_after_seconds((uint32_t)(cb->cb_arg));
        break;

        case PM_STANDBY:
        case PM_SHUTDOWN:
        break;

        default:
        break;
    }

    return 0;
}

static int lp_test_gpio_async_interrupt(struct pm_callback_s *cb, enum pm_state_e state)
{
    uint32_t wk_pin = (uint32_t)(cb->cb_arg);
    gpio_dev_t *gpio_dev = GPIO_INSTANCE(wk_pin);

    switch (state) {
        case PM_LIGHT_SLEEP:
            gpio_async_int_enable(gpio_dev, wk_pin, true);
        break;

        case PM_DEEP_SLEEP:
            gpio_async_int_enable(gpio_dev, wk_pin, true);
        break;

        case PM_STANDBY:
        case PM_SHUTDOWN:
        break;

        default:
        break;
    }

    return 0;
}

static void lp_test_reset_spsx_for_display(void)
{
#if !(defined(CONFIG_BOARD_E3_176_REF) || defined(CONFIG_BOARD_E3_144_REF))
    for (uint32_t i = 7; i <= 10; i++) {
        ASSERT(i <= ARRAY_SIZE(reset_sigs_case0));
        reset_sigs_case0[i].sleep_mode_reset = true;
        reset_sigs_case0[i].hib_mode_reset = true;
    }
#endif
}

static void lp_test_reset_spsx_iram2_3_for_176_144(void)
{
    /* no sp/sx, need assert reset sig */
    for (uint32_t i = 2; i <= 5; i++) {
        RMWREG32(APB_RSTGEN_SF_BASE + 0x1a04 + i * 0x8, 0, 3, 0);
    }

    /* iramc2 & 3 clock can't gate */
    ckgen_xcg_set_gating_state(APB_CKGEN_SF_BASE, CKGEN_PCG_ID(2), CKGEN_CG_STATE(1, 1, 1));
    ckgen_xcg_set_gating_state(APB_CKGEN_SF_BASE, CKGEN_PCG_ID(86), CKGEN_CG_STATE(1, 1, 1));
    ckgen_xcg_set_gating_state(APB_CKGEN_SF_BASE, CKGEN_PCG_ID(3), CKGEN_CG_STATE(1, 1, 1));
    ckgen_xcg_set_gating_state(APB_CKGEN_SF_BASE, CKGEN_PCG_ID(87), CKGEN_CG_STATE(1, 1, 1));
}

static void lp_test_core_pd_wakeup_str(void)
{
    /* all core power gated */
    g_smc_test_cfg.sf_mix.hib_pd_enable = true;
    g_smc_test_cfg.sp_mix.hib_pd_enable = true;
    g_smc_test_cfg.sx_mix.hib_pd_enable = true;
    g_smc_test_cfg.gama1_pwr.hib_pd_enable = true;

    /* ap & disp power gated */
    g_smc_test_cfg.ap_disp_ss.hib_pd_enable = true;

    /* hib mode core ram power down */
    g_smc_test_cfg.cr5_sf.hib_ram_pd_enable = true;
    g_smc_test_cfg.cr5_sf.hib_ram_lp_setting = RAM_LP_PG_EN |RAM_LP_RET1N | RAM_LP_RET2N;

    g_smc_test_cfg.cr5_sp.hib_ram_pd_enable = true;
    g_smc_test_cfg.cr5_sp.hib_ram_lp_setting = RAM_LP_PG_EN |RAM_LP_RET1N | RAM_LP_RET2N;

    g_smc_test_cfg.cr5_sx.hib_ram_pd_enable = true;
    g_smc_test_cfg.cr5_sx.hib_ram_lp_setting = RAM_LP_PG_EN |RAM_LP_RET1N | RAM_LP_RET2N;

    g_smc_test_cfg.gama1_ram.hib_ram_pd_enable = true;
    g_smc_test_cfg.gama1_ram.hib_ram_lp_setting = RAM_LP_PG_EN |RAM_LP_RET1N | RAM_LP_RET2N;

    g_smc_test_cfg.saf_misc.hib_ram_pd_enable = true;
    g_smc_test_cfg.saf_misc.hib_ram_lp_setting = RAM_LP_PG_EN |RAM_LP_RET1N | RAM_LP_RET2N;

    g_smc_test_cfg.ap_ram.hib_ram_pd_enable = true;
    g_smc_test_cfg.ap_ram.hib_ram_lp_setting = RAM_LP_PG_EN |RAM_LP_RET1N | RAM_LP_RET2N;

    g_smc_test_cfg.disp_ram.hib_ram_pd_enable = true;
    g_smc_test_cfg.disp_ram.hib_ram_lp_setting = RAM_LP_PG_EN |RAM_LP_RET1N | RAM_LP_RET2N;

#if !(defined(CONFIG_BOARD_E3_176_REF) || defined(CONFIG_BOARD_E3_144_REF))
    /* after wakeup, need reset core */
    for (uint32_t i = 6; i <= 14; i++) {
        ASSERT(i <= ARRAY_SIZE(reset_sigs_case0));
        reset_sigs_case0[i].hib_mode_reset = true;
    }
#else
    for (uint32_t i = 2; i <= 6; i++) {
        ASSERT(i <= ARRAY_SIZE(reset_sigs_case0));
        reset_sigs_case0[i].hib_mode_reset = true;
    }
#endif
}

/*
sleep: core/peripheral clock gated, pll gated
*/
static void lp_test_case_config_0(void)
{
    /* wakeup: uart6 */
    g_smc_ckgen_test_cfg.enable_clk     = uart6_clks;
    g_smc_ckgen_test_cfg.enable_clk_num = ARRAY_SIZE(uart6_clks);
    g_smc_test_cfg.wk_irq               = uart6_irqs;
    g_smc_test_cfg.wk_irq_num           = ARRAY_SIZE(uart6_irqs);
    g_smc_rstgen_test_cfg.smc_reset     = reset_sigs_case0;
    g_smc_rstgen_test_cfg.smc_reset_num = ARRAY_SIZE(reset_sigs_case0);

    /* sleep */
    /* PLL2/PLL3 keep active, uart6 use PLL3, bus & core use PLL2 */
    g_smc_ckgen_test_cfg.pll1.sleep_mode_disable = true;
    g_smc_ckgen_test_cfg.pll4.sleep_mode_disable = true;
    g_smc_ckgen_test_cfg.pll5.sleep_mode_disable = true;
    g_smc_ckgen_test_cfg.lvds.sleep_mode_disable = true;

    /* saf & ap xtal keep active */

    /* hibernate */
    /* PLL power down, except pll2 & pll3, ap xtal gated, all power switch gated */
    g_smc_ckgen_test_cfg.pll1.hib_mode_disable = true;
    g_smc_ckgen_test_cfg.pll4.hib_mode_disable = true;
    g_smc_ckgen_test_cfg.pll5.hib_mode_disable = true;
    g_smc_ckgen_test_cfg.lvds.hib_mode_disable = true;
    g_smc_ckgen_test_cfg.pll1.hib_mode_pd = true;
    g_smc_ckgen_test_cfg.pll4.hib_mode_pd = true;
    g_smc_ckgen_test_cfg.pll5.hib_mode_pd = true;
    g_smc_ckgen_test_cfg.lvds.hib_mode_pd = true;

    /* ap domain xtal hib mode gated */
    g_smc_ckgen_test_cfg.ap.hib_mode_disable = true;
}

static void lp_test_case_config_100(void)
{
    lp_test_case_config_0();

    /* wakeup: uart7 */
    g_smc_ckgen_test_cfg.enable_clk     = uart7_clks;
    g_smc_ckgen_test_cfg.enable_clk_num = ARRAY_SIZE(uart7_clks);
    g_smc_test_cfg.wk_irq               = uart7_irqs;
    g_smc_test_cfg.wk_irq_num           = ARRAY_SIZE(uart7_irqs);
}

static void lp_test_case_config_200(void)
{
    lp_test_case_config_0();
    lp_test_reset_spsx_for_display();

    /* wakeup: uart4 */
    g_smc_ckgen_test_cfg.enable_clk     = uart4_clks;
    g_smc_ckgen_test_cfg.enable_clk_num = ARRAY_SIZE(uart4_clks);
    g_smc_test_cfg.wk_irq               = uart4_irqs;
    g_smc_test_cfg.wk_irq_num           = ARRAY_SIZE(uart4_irqs);
}

static void lp_test_case_config_3(void);

static void lp_test_case_config_103(void)
{
    lp_test_case_config_3();

    /* wakeup: uart7 */
    g_smc_ckgen_test_cfg.enable_clk     = uart7_clks;
    g_smc_ckgen_test_cfg.enable_clk_num = ARRAY_SIZE(uart7_clks);
    g_smc_test_cfg.wk_irq               = uart7_irqs;
    g_smc_test_cfg.wk_irq_num           = ARRAY_SIZE(uart7_irqs);
}

static void lp_test_case_config_203(void)
{
    lp_test_case_config_3();
    lp_test_reset_spsx_for_display();

    /* wakeup: uart4 */
    g_smc_ckgen_test_cfg.enable_clk     = uart4_clks;
    g_smc_ckgen_test_cfg.enable_clk_num = ARRAY_SIZE(uart4_clks);
    g_smc_test_cfg.wk_irq               = uart4_irqs;
    g_smc_test_cfg.wk_irq_num           = ARRAY_SIZE(uart4_irqs);
}

static void lp_test_case_config_3(void)
{
    lp_test_case_config_0();

    /* sleep */
    g_smc_ckgen_test_cfg.pll2.sleep_mode_disable = true;
    g_smc_ckgen_test_cfg.pll3.sleep_mode_disable = true;

    /* saf & ap xtal gated */
    g_smc_ckgen_test_cfg.saf.sleep_mode_disable = true;
    g_smc_ckgen_test_cfg.ap.sleep_mode_disable = true;

    /* hibernate */
    g_smc_ckgen_test_cfg.pll2.hib_mode_disable = true;
    g_smc_ckgen_test_cfg.pll3.hib_mode_disable = true;
    g_smc_ckgen_test_cfg.pll2.hib_mode_pd = true;
    g_smc_ckgen_test_cfg.pll3.hib_mode_pd = true;

    /* hib mode all xtal gated */
    g_smc_ckgen_test_cfg.saf.hib_mode_disable = true;
    g_smc_ckgen_test_cfg.ap.hib_mode_disable = true;

    lp_test_core_pd_wakeup_str();

    /* all pll & xtal disable, so need switch bus/core/wakeup-ip clocks to rc24m */
    lp_test_cb.suspend = lp_test_bus_core_uart6_clock_rc24m;
    lp_test_cb.resume = lp_test_bus_core_uart6_clock_resume;
}

static void lp_test_case_config_4(void)
{
    lp_test_case_config_3();

    /* sleep */
    g_smc_ckgen_test_cfg.pll1.sleep_mode_pd = true;
    g_smc_ckgen_test_cfg.pll2.sleep_mode_pd = true;
    g_smc_ckgen_test_cfg.pll3.sleep_mode_pd = true;
    g_smc_ckgen_test_cfg.pll4.sleep_mode_pd = true;
    g_smc_ckgen_test_cfg.pll5.sleep_mode_pd = true;
    g_smc_ckgen_test_cfg.lvds.sleep_mode_pd = true;

    /* hibernate */
    /* AP domain power off */
    // g_smc_pmu_test_cfg.hib.pwr_ctrl1 = false;
    // g_smc_pmu_test_cfg.hib.ap_power_on = false;
}

static void lp_test_case_config_104(void)
{
    lp_test_case_config_4();

    /* wakeup: uart7 */
    g_smc_ckgen_test_cfg.enable_clk     = uart7_clks;
    g_smc_ckgen_test_cfg.enable_clk_num = ARRAY_SIZE(uart7_clks);
    g_smc_test_cfg.wk_irq               = uart7_irqs;
    g_smc_test_cfg.wk_irq_num           = ARRAY_SIZE(uart7_irqs);
}

static void lp_test_case_config_204(void)
{
    lp_test_case_config_4();
    lp_test_reset_spsx_for_display();

    /* wakeup: uart4 */
    g_smc_ckgen_test_cfg.enable_clk     = uart4_clks;
    g_smc_ckgen_test_cfg.enable_clk_num = ARRAY_SIZE(uart4_clks);
    g_smc_test_cfg.wk_irq               = uart4_irqs;
    g_smc_test_cfg.wk_irq_num           = ARRAY_SIZE(uart4_irqs);
}

static void lp_test_case_config_5(void)
{
    lp_test_case_config_4();

    /* sleep */

    /* hibernate */
    /* SAF domain can't power off */
    // g_smc_pmu_test_cfg.hib.pwr_ctrl0 = false;

    g_smc_test_cfg.wk_irq               = uart6_rtc1_irqs;
    g_smc_test_cfg.wk_irq_num           = ARRAY_SIZE(uart6_rtc1_irqs);

    lp_test_cb.suspend = lp_test_config_rtc_wakeup_timer;
    lp_test_cb.resume = lp_test_bus_core_uart6_clock_resume;
    /* stay hib time, unit: second */
    lp_test_cb.cb_arg = (void *)10;

    /* use rtc timer wakeup from hib, disable rc24m */
    g_smc_test_cfg.hib_disable_rc24m = true;
}

static void lp_test_case_config_105(void)
{
    lp_test_case_config_5();

    /* wakeup: sleep-uart7 & hib-rtc1 */
    g_smc_ckgen_test_cfg.enable_clk     = uart7_clks;
    g_smc_ckgen_test_cfg.enable_clk_num = ARRAY_SIZE(uart7_clks);
    g_smc_test_cfg.wk_irq               = uart7_rtc1_irqs;
    g_smc_test_cfg.wk_irq_num           = ARRAY_SIZE(uart7_rtc1_irqs);
}

static void lp_test_case_config_205(void)
{
    lp_test_case_config_5();
    lp_test_reset_spsx_for_display();

    /* wakeup: sleep-uart4 & hib-rtc1 */
    g_smc_ckgen_test_cfg.enable_clk     = uart4_clks;
    g_smc_ckgen_test_cfg.enable_clk_num = ARRAY_SIZE(uart4_clks);
    g_smc_test_cfg.wk_irq               = uart4_rtc1_irqs;
    g_smc_test_cfg.wk_irq_num           = ARRAY_SIZE(uart4_rtc1_irqs);
}

static void async_gpio_irq_handler(struct gpio_dev *dev, int pin, void *arg)
{
    printf("%s: pin %d irq\n", __func__, pin);

    gpio_async_int_enable(dev, pin, false);
}

static void lp_test_case_config_6(uint32_t wk_pin)
{
    gpio_dev_t *gpio_dev;
    pinctrl_t *pinctrl;

    /* sleep */
    g_smc_ckgen_test_cfg.enable_clk     = NULL;
    g_smc_ckgen_test_cfg.enable_clk_num = 0;
    g_smc_test_cfg.wk_irq               = gpio_irqs;
    g_smc_test_cfg.wk_irq_num           = ARRAY_SIZE(gpio_irqs);
    g_smc_rstgen_test_cfg.smc_reset     = reset_sigs_case0;
    g_smc_rstgen_test_cfg.smc_reset_num = ARRAY_SIZE(reset_sigs_case0);

    g_smc_ckgen_test_cfg.pll1.sleep_mode_disable = true;
    g_smc_ckgen_test_cfg.pll2.sleep_mode_disable = true;
    g_smc_ckgen_test_cfg.pll3.sleep_mode_disable = true;
    g_smc_ckgen_test_cfg.pll4.sleep_mode_disable = true;
    g_smc_ckgen_test_cfg.pll5.sleep_mode_disable = true;
    g_smc_ckgen_test_cfg.lvds.sleep_mode_disable = true;

    g_smc_ckgen_test_cfg.pll1.sleep_mode_pd = true;
    g_smc_ckgen_test_cfg.pll2.sleep_mode_pd = true;
    g_smc_ckgen_test_cfg.pll3.sleep_mode_pd = true;
    g_smc_ckgen_test_cfg.pll4.sleep_mode_pd = true;
    g_smc_ckgen_test_cfg.pll5.sleep_mode_pd = true;
    g_smc_ckgen_test_cfg.lvds.sleep_mode_pd = true;

    /* saf & ap xtal disable */
    g_smc_ckgen_test_cfg.saf.sleep_mode_disable = true;
    g_smc_ckgen_test_cfg.ap.sleep_mode_disable = true;

    /* hibernate */
    g_smc_ckgen_test_cfg.pll1.hib_mode_disable = true;
    g_smc_ckgen_test_cfg.pll2.hib_mode_disable = true;
    g_smc_ckgen_test_cfg.pll3.hib_mode_disable = true;
    g_smc_ckgen_test_cfg.pll4.hib_mode_disable = true;
    g_smc_ckgen_test_cfg.pll5.hib_mode_disable = true;
    g_smc_ckgen_test_cfg.lvds.hib_mode_disable = true;

    g_smc_ckgen_test_cfg.pll1.hib_mode_pd = true;
    g_smc_ckgen_test_cfg.pll2.hib_mode_pd = true;
    g_smc_ckgen_test_cfg.pll3.hib_mode_pd = true;
    g_smc_ckgen_test_cfg.pll4.hib_mode_pd = true;
    g_smc_ckgen_test_cfg.pll5.hib_mode_pd = true;
    g_smc_ckgen_test_cfg.lvds.hib_mode_pd = true;

    g_smc_ckgen_test_cfg.saf.hib_mode_disable = true;
    g_smc_ckgen_test_cfg.ap.hib_mode_disable = true;

    g_smc_test_cfg.hib_disable_rc24m = true;

    /* config gpio async interrupt, high level */
    gpio_dev = GPIO_INSTANCE(wk_pin);
    pinctrl = PINCTRL_INSTANCE(wk_pin);

    pinctrl_config(pinctrl, wk_pin, PINCTRL_CONFIG_FUNCTION, PIN_MUX_ALT0);
    pinctrl_config(pinctrl, wk_pin, PINCTRL_CONFIG_DRIVE, PIN_PUSH_PULL);
    pinctrl_config(pinctrl, wk_pin, PINCTRL_CONFIG_PULL, PIN_PULL_DOWN);

    gpio_config(gpio_dev, wk_pin, GPIO_INTR_HIGH_LEVEL);
    gpio_attach_async_irq(gpio_dev, wk_pin, async_gpio_irq_handler, NULL);

    lp_test_cb.suspend = lp_test_gpio_async_interrupt;
    lp_test_cb.resume = NULL;
    lp_test_cb.cb_arg = (void *)wk_pin;
}

static void lp_test_case_config_106(uint32_t wk_pin)
{
    lp_test_case_config_6(wk_pin);

#ifdef CONFIG_SBC
    g_smc_ckgen_test_cfg.enable_clk     = sbc_clk;
    g_smc_ckgen_test_cfg.enable_clk_num = ARRAY_SIZE(sbc_clk);
#endif
}

static void lp_test_case_config_206(uint32_t wk_pin)
{
    lp_test_case_config_6(wk_pin);
    lp_test_reset_spsx_for_display();
}

static void lp_test_case_config_300(void)
{
    lp_test_case_config_0();

    g_smc_ckgen_test_cfg.enable_clk     = uart_clks_176_144;
    g_smc_ckgen_test_cfg.enable_clk_num = ARRAY_SIZE(uart_clks_176_144);
    g_smc_test_cfg.wk_irq               = uart_irq_176_144;
    g_smc_test_cfg.wk_irq_num           = ARRAY_SIZE(uart_irq_176_144);
}

static int lp_test_use_rtc1_timer_wakeup(struct pm_callback_s *cb, enum pm_state_e state)
{
    switch (state) {
        case PM_LIGHT_SLEEP:
        case PM_DEEP_SLEEP:
            lp_rtc_wakeup_after_seconds((uint32_t)(cb->cb_arg));
        break;

        case PM_STANDBY:
        case PM_SHUTDOWN:
        break;

        default:
        break;
    }

    return 0;
}

static void lp_test_case_config_301(void)
{
    lp_test_case_config_0();

    g_smc_ckgen_test_cfg.enable_clk     = NULL;
    g_smc_ckgen_test_cfg.enable_clk_num = 0;
    g_smc_test_cfg.wk_irq               = rtc1_irq;
    g_smc_test_cfg.wk_irq_num           = ARRAY_SIZE(rtc1_irq);

    /* sleep */
    g_smc_ckgen_test_cfg.pll2.sleep_mode_disable = true;
    g_smc_ckgen_test_cfg.pll3.sleep_mode_disable = true;

    /* saf & ap xtal gated */
    g_smc_ckgen_test_cfg.saf.sleep_mode_disable = true;
    g_smc_ckgen_test_cfg.ap.sleep_mode_disable = true;

    /* hibernate */
    g_smc_ckgen_test_cfg.pll2.hib_mode_disable = true;
    g_smc_ckgen_test_cfg.pll3.hib_mode_disable = true;
    g_smc_ckgen_test_cfg.pll2.hib_mode_pd = true;
    g_smc_ckgen_test_cfg.pll3.hib_mode_pd = true;

    /* hib mode all xtal gated */
    g_smc_ckgen_test_cfg.saf.hib_mode_disable = true;
    g_smc_ckgen_test_cfg.ap.hib_mode_disable = true;

    lp_test_cb.suspend = lp_test_use_rtc1_timer_wakeup;
    lp_test_cb.resume = NULL;
    /* stay hib time, unit: second */
    lp_test_cb.cb_arg = (void *)10;

    /* use rtc timer wakeup from hib, disable rc24m */
    g_smc_test_cfg.hib_disable_rc24m = true;
}

static void lp_test_case_config_302(uint32_t wk_pin)
{
    lp_test_case_config_6(wk_pin);

    lp_test_core_pd_wakeup_str();
}

static void lp_test_case_config_303(void)
{
    lp_test_case_config_0();

    g_smc_ckgen_test_cfg.enable_clk     = canfd_clks_176_144;
    g_smc_ckgen_test_cfg.enable_clk_num = ARRAY_SIZE(canfd_clks_176_144);
    g_smc_test_cfg.wk_irq               = canfd_irq_176_144;
    g_smc_test_cfg.wk_irq_num           = ARRAY_SIZE(canfd_irq_176_144);
}

/**
 * @brief change bus/core slice to RC24M
 *
 * SAF domain: BUS: cr5-sf, cr5-sp, CORE: cr5-sx
 * AP domain: BUS: ap-bus
 *
 * ap domain disp-bus, seip-bus not touch
 */
static void lp_test_bus_core_clk_switch_rc24m(void)
{
    lp_ckgen_node_switch_rc24m(&g_ckgen_bus_cr5_sf_div_root);

#ifdef CONFIG_PART_SP
    lp_ckgen_node_switch_rc24m(&g_ckgen_bus_cr5_sp_div_root);
#endif

#ifdef CONFIG_PART_SX
    lp_ckgen_node_switch_rc24m(&g_ckgen_core_cr5_sx);
#endif

    lp_ckgen_node_switch_rc24m(&g_ckgen_bus_ap_bus_div_root);
}

static void lp_test_bus_core_clk_switch_config(void)
{
    lp_ckgen_node_switch_config(&g_ckgen_bus_cr5_sf_div_root);

#ifdef CONFIG_PART_SP
    lp_ckgen_node_switch_config(&g_ckgen_bus_cr5_sp_div_root);
#endif

#ifdef CONFIG_PART_SX
    lp_ckgen_node_switch_config(&g_ckgen_core_cr5_sx);
#endif

    lp_ckgen_node_switch_config(&g_ckgen_bus_ap_bus_div_root);
}

static int power_cfg_demo_main(int argc, char *argv[])
{
    uint32_t test_case = 0;

    if (argc > 0) {
        test_case = atoi(argv[0]);
    }

    printf("Low power config demo, use case %d config\n", test_case);

    /* bypass lbist */
    RMWREG32(0xf05f1290, 0, 1, 0);

    switch (test_case) {
        /* E3_BGA324_EVB
        * Sleep: all clock/pll gated, keep core/bus/wakeup-ip clock
          Hibernate: PLL power down, except pll2 & pll3, ap xtal gated,
                     all power switch gated, core ram retention */
        case 0:
            lp_test_case_config_0();
        break;

        /* base case0, addition
        * Sleep: all pll/xtal gated, switch necessary clock to rc24m
          Hibernate: all pll power down, all xtal gated, switch necessary clock to rc24m
                     all power switch gated, core ram power down */
        case 3:
            lp_test_case_config_3();
        break;

        /* base case3, addition
        * Sleep: all pll power down
          Hibernate:  AP domain power off, SAF domain power on */
        /*!!! [ fixed ] ap domain pd, when wakeup, ap domain do ap_por_b,
            PLL in ap domain be reset, when sf config ap bus clock will hang. */
        case 4:
            lp_test_case_config_4();
        break;

        /* base case4, addition
        * Sleep: n/a
          Hibernate: AP domain power off, use RTC timer wakeup
          SAF domain can't power off, because smc can't work */
        case 5:
            lp_test_case_config_5();
        break;

        /* E3_BGA324_EVB, gpio_a0
        * Sleep: all clock/pll gated, use async gpio interrupt wakeup
          Hibernate: all clock disalbe, include rc24m */
        case 6:
            lp_test_case_config_6(GPIO_A0);
        break;

        /* ------------------------------------------------------------------------------- */

        /* E3-BGA324-Gateway
          case0 test on gateway */
        case 100:
            lp_test_case_config_100();
        break;

        /* E3-BGA324-Gateway
          case3 test on gateway */
        case 103:
            lp_test_case_config_103();
        break;

        /* E3-BGA324-Gateway
          case4 test on gateway */
        case 104:
            lp_test_case_config_104();
        break;

        /* E3-BGA324-Gateway
          case5 test on gateway */
        case 105:
            lp_test_case_config_105();
        break;

        /* E3-BGA324-Gateway, gpio_a0
          case6 test on gateway */
        case 106:
            lp_test_case_config_106(GPIO_A0);
        break;

        /* ------------------------------------------------------------------------------- */

        /* E3-BGA324-Disp
          case0 test on display */
        case 200:
            lp_test_case_config_200();
        break;

        /* E3-BGA324-Disp
          case3 test on display */
        case 203:
            lp_test_case_config_203();
        break;

        /* E3-BGA324-Disp
          case4 test on display */
        case 204:
            lp_test_case_config_204();
        break;

        /* E3-BGA324-Disp
          case5 test on display */
        case 205:
            lp_test_case_config_205();
        break;

        /* E3-BGA324-Disp, gpio_a0
          case6 test on display */
        case 206:
            lp_test_case_config_206(GPIO_A0);
        break;

        /* ------------------------------------------------------------------------------- */
        /* E3 LQFP176/144 */
        /* uart wakeup */
        case 300:
            lp_test_case_config_300();
        break;

        /* rtc timer wakeup */
        case 301:
            lp_test_case_config_301();
        break;

        /* gpio aint, gpio_a0 */
        case 302:
            lp_test_case_config_302(GPIO_A0);
        break;

        /* canfd3, canfd4 */
        case 303:
            lp_test_case_config_303();
        break;

        default:
            printf("Unknown case %d\n", test_case);
            return CLI_FALSE;
    }

    if (test_case >= 200 && test_case < 300) {
        lp_test_reset_spsx_for_display();
    }

    sdrv_smc_ckgen_config(&g_smc_ckgen_test_cfg);
    sdrv_smc_rstgen_config(&g_smc_rstgen_test_cfg);
    sdrv_smc_pmu_config(&g_smc_pmu_test_cfg);
    sdrv_smc_init(&g_smc_test_cfg);

    pm_register(&lp_test_cb);

#ifdef CONFIG_SBC
    pm_register(&lp_test_sbc_cb);
#endif

#if (defined(CONFIG_BOARD_E3_176_REF) || defined(CONFIG_BOARD_E3_144_REF))
    lp_test_reset_spsx_iram2_3_for_176_144();
#endif

    printf("Now use pm_request 2/3 enter sleep/hib\n");

    return CLI_FALSE;
}

static pin_settings_config_t soc_swm_pins[] = {
    /* Pin 103, GPIO_S2, Mux:SMC.SWM0 */
    {
        .pin_index         =  GPIO_S2,
        .mux               =  PIN_MUX_ALT2,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_NOPULL,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_SLOW,
        .input_select      =  PIN_IS_CMOS,
        .data_direction    =  PIN_OUTPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_NORMAL,
        .mode_select       =  PIN_MODE_DIGITAL,
    },

    /* Pin 104, GPIO_S3, Mux:SMC.SWM1 */
    {
        .pin_index         =  GPIO_S3,
        .mux               =  PIN_MUX_ALT2,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_NOPULL,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_SLOW,
        .input_select      =  PIN_IS_CMOS,
        .data_direction    =  PIN_OUTPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_NORMAL,
        .mode_select       =  PIN_MODE_DIGITAL,
    },

    /* Pin 105, GPIO_S4, Mux:SMC.SWM2 */
    {
        .pin_index         =  GPIO_S4,
        .mux               =  PIN_MUX_ALT2,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_NOPULL,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_SLOW,
        .input_select      =  PIN_IS_CMOS,
        .data_direction    =  PIN_INPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_NORMAL,
        .mode_select       =  PIN_MODE_DIGITAL,
    },

    /* Pin 106, GPIO_S5, Mux:SMC.SWM3 */
    {
        .pin_index         =  GPIO_S5,
        .mux               =  PIN_MUX_ALT2,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_NOPULL,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_SLOW,
        .input_select      =  PIN_IS_CMOS,
        .data_direction    =  PIN_INPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_NORMAL,
        .mode_select       =  PIN_MODE_DIGITAL,
    },
};

static int smc_swm_config_main(int argc, char *argv[])
{
    for (unsigned int i = 0; i < ARRAY_SIZE(soc_swm_pins); i++) {
        pin_settings_config_t *config = &soc_swm_pins[i];
        pinctrl_t *pinctrl = (config->pin_index < TAISHAN_SAFETY_PIN_NUM) ? &g_pinctrl_safety : &g_pinctrl_ap;

        pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_FUNCTION, config->mux);
        pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_DRIVE, config->open_drain);
        pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_PULL, config->pull_config);
        pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_DRIVE_STRENGTH, config->drive_strength);
        pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_SLEW_RATE, config->slew_rate);
        pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_INPUT_SCHMITT, config->input_select);
        pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_FORCE_INPUT, config->force_input);
    }

    printf("Config GPIO_S2, GPIO_S3, GPIO_S4, GPIO_S5 as SMC.SWM function.\n");
    printf("0b0001:Run, 0b0010:Sleep, 0b0100:Hibernate, 0b1000:RTC\n");

    return CLI_FALSE;
}

CLI_CMD("power_cfg_demo", "\r\npower_cfg_demo:\r\n Config different low power case \r\n", power_cfg_demo_main);
CLI_CMD("smc_swm_pin", "\r\nsmc_swm_pin:\r\n Smc SWM pinmux config\r\n", smc_swm_config_main);

#endif /* CONFIG_SDRV_SMC */
