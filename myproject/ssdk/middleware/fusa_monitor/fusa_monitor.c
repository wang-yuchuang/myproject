/**
 * @file sem_monitor.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <stdio.h>
#include <cmsis_os2.h>
#include <reg.h>

#include "driver/scr.h"
#include "driver/irq.h"
#include "irq_num.h"
#include "param.h"
#include "regs_base.h"
#include "scr_cfg.h"
#include "scr_hw.h"
#include "taishan/sem/sdrv_sem.h"
#include "sem_hw.h"

#include "fusa_monitor.h"

static const scr_signal_t g_sem_irq_sig[] = {
    SCR_SF_REMAP_CR5_SF_AW_IRQ_EN,
    SCR_SF_REMAP_CR5_SF_AR_IRQ_EN,
    SCR_SF_REMAP_CR5_SX_AW0_IRQ_EN,
    SCR_SF_REMAP_CR5_SX_AR0_IRQ_EN,
    SCR_SF_REMAP_CR5_SX_AW1_IRQ_EN,
    SCR_SF_REMAP_CR5_SX_AR1_IRQ_EN,
    SCR_SF_REMAP_CR5_SP_AW0_IRQ_EN,
    SCR_SF_REMAP_CR5_SP_AR0_IRQ_EN,
    SCR_SF_REMAP_CR5_SP_AW1_IRQ_EN,
    SCR_SF_REMAP_CR5_SP_AR1_IRQ_EN,
    SCR_SF_IRAM_MUX_UNC_IRQ_EN,
    SCR_SF_AHB2APB1_COR_IRQ_EN,
    SCR_SF_AHB2APB1_UNCOR_IRQ_EN,
    SCR_SF_AHB2APB2_COR_IRQ_EN,
    SCR_SF_AHB2APB2_UNCOR_IRQ_EN,
    SCR_SF_AHB2APB3_COR_IRQ_EN,
    SCR_SF_AHB2APB3_UNCOR_IRQ_EN,
    SCR_SF_AHB2APB4_COR_IRQ_EN,
    SCR_SF_AHB2APB4_UNCOR_IRQ_EN,
    SCR_SF_APB_APBMUX1_MST_DST_IRQ_ENB,
    SCR_SF_APB_APBMUX1_SLV_SRC_IRQ_ENB,
    SCR_SF_APB_MAC_SP_MST_DST_IRQ_ENB,
    SCR_SF_APB_MAC_SP_SLV_SRC_IRQ_ENB,
    SCR_SF_AAPB_MAC_SRC_IRQ_ENB,
    SCR_SF_AAPB_MAC_DST_IRQ_ENB,
    SCR_SF_APB_LDO_DIG_MST_DST_IRQ_ENB,
    SCR_SF_APB_LDO_DIG_SLV_SRC_IRQ_ENB,
    SCR_SF_APB_DCDC1_MST_DST_IRQ_ENB,
    SCR_SF_APB_DCDC1_SLV_SRC_IRQ_ENB,
    SCR_SF_APB_SEC_STORAGE1_MST_DST_IRQ_ENB,
    SCR_SF_APB_SEC_STORAGE1_SLV_SRC_IRQ_ENB,
    SCR_SF_AAPB_XSPI1A_SRC_IRQ_ENB,
    SCR_SF_AAPB_XSPI1A_DST_IRQ_ENB,
    SCR_SF_AAPB_XSPI1B_SRC_IRQ_ENB,
    SCR_SF_AAPB_XSPI1B_DST_IRQ_ENB,
    SCR_SF_AAPB_XSPI2A_SRC_IRQ_ENB,
    SCR_SF_AAPB_XSPI2A_DST_IRQ_ENB,
    SCR_SF_AAPB_XSPI2B_SRC_IRQ_ENB,
    SCR_SF_AAPB_XSPI2B_DST_IRQ_ENB,
    SCR_SF_AAPB_FLEXRAY1_SRC_IRQ_ENB,
    SCR_SF_AAPB_FLEXRAY1_DST_IRQ_ENB,
    SCR_SF_AAPB_FLEXRAY2_SRC_IRQ_ENB,
    SCR_SF_AAPB_FLEXRAY2_DST_IRQ_ENB,
    SCR_SF_APB_APBMUX3_SLV_SRC_IRQ_ENB,
    SCR_SF_APB_APBMUX3_MST_DST_IRQ_ENB,
    SCR_SF_APB_APBMUX4_SLV_SRC_IRQ_ENB,
    SCR_SF_APB_APBMUX4_MST_DST_IRQ_ENB,
    SCR_SF_MPC_XSPI1A_SRC_IRQ_ENB,
    SCR_SF_MPC_XSPI1A_DST_IRQ_ENB,
    SCR_SF_MPC_XSPI1B_SRC_IRQ_ENB,
    SCR_SF_MPC_XSPI1B_DST_IRQ_ENB,
    SCR_SF_MPC_XSPI2A_SRC_IRQ_ENB,
    SCR_SF_MPC_XSPI2A_DST_IRQ_ENB,
    SCR_SF_MPC_XSPI2B_SRC_IRQ_ENB,
    SCR_SF_MPC_XSPI2B_DST_IRQ_ENB,
    SCR_SF_MPC_CR5_SF_SRC_IRQ_ENB,
    SCR_SF_MPC_CR5_SF_DST_IRQ_ENB,
    SCR_SF_MPC_CR5_SX0_SRC_IRQ_ENB,
    SCR_SF_MPC_CR5_SX0_DST_IRQ_ENB,
    SCR_SF_MPC_CR5_SX1_SRC_IRQ_ENB,
    SCR_SF_MPC_CR5_SX1_DST_IRQ_ENB,
    SCR_SF_MPC_CR5_SP0_SRC_IRQ_ENB,
    SCR_SF_MPC_CR5_SP0_DST_IRQ_ENB,
    SCR_SF_MPC_CR5_SP1_SRC_IRQ_ENB,
    SCR_SF_MPC_CR5_SP1_DST_IRQ_ENB,
    SCR_SF_AAPB_PLL1_SRC_IRQ_ENB,
    SCR_SF_AAPB_PLL1_DST_IRQ_ENB,
    SCR_SF_AAPB_PLL2_SRC_IRQ_ENB,
    SCR_SF_AAPB_PLL2_DST_IRQ_ENB,
    SCR_SF_AAPB_PLL3_SRC_IRQ_ENB,
    SCR_SF_AAPB_PLL3_DST_IRQ_ENB,
    SCR_SF_BTI_SF_M0_UNCOR_IRQ_EN,
    SCR_SF_BTI_SF_P0_UNCOR_IRQ_EN,
    SCR_SF_BTI_SF_AHB_UNCOR_IRQ_EN,
    SCR_SF_BTI_SX_M0_UNCOR_IRQ_EN,
    SCR_SF_BTI_SX_P0_UNCOR_IRQ_EN,
    SCR_SF_BTI_SX_M1_UNCOR_IRQ_EN,
    SCR_SF_BTI_SX_P1_UNCOR_IRQ_EN,
    SCR_SF_BTI_SX_AHB_UNCOR_IRQ_EN,
    SCR_SF_BTI_DISPSF_UNCOR_IRQ_EN,
    SCR_SF_BTI_DISPSF_COR_IRQ_EN,
    SCR_SF_BTI_APSF_UNCOR_IRQ_EN,
    SCR_SF_BTI_APSF_COR_IRQ_EN,
    SCR_SF_BTI_SP_M0_UNCOR_IRQ_EN,
    SCR_SF_BTI_SP_P0_UNCOR_IRQ_EN,
    SCR_SF_BTI_SP0_AHB_UNCOR_IRQ_EN,
    SCR_SF_BTI_SP_M1_UNCOR_IRQ_EN,
    SCR_SF_BTI_SP_P1_UNCOR_IRQ_EN
};

static inline void writel_set(unsigned int addr, unsigned int bits)
{
    volatile unsigned int *p = (unsigned int *)addr;
    unsigned int val;

    val = *p;
    val |= bits;
    *p = val;
}

static void scr_err_irq_enable(void)
{
    for (size_t cnt =  0; cnt < ARRAY_SIZE(g_sem_irq_sig); cnt++) {
        scr_set(&g_scr_sf, &g_sem_irq_sig[cnt], 1);
    }
}

static void ldo_dcdc_err_irq_enable(void)
{
    /* enable sem irq for ldo */
    writel_set((APB_LDO_DIG_BASE + 0x38), 0x7);
    writel_set((APB_LDO_DIG_BASE + 0x3c), 0x7);
    /* enable sem irq for dcdc */
    writel_set((APB_DCDC1_BASE + 0x50), 0x7);
    writel_set((APB_DCDC1_BASE + 0x54), 0x7);
}

static void i2c_err_irq_enable(void)
{
    /* enable sem irq for i2c */
    writel_set((APB_I2C1_BASE + 0x3c), 0x67F001F);
    writel_set((APB_I2C2_BASE + 0x3c), 0x67F001F);
    writel_set((APB_I2C3_BASE + 0x3c), 0x67F001F);
    writel_set((APB_I2C4_BASE + 0x3c), 0x67F001F);
    writel_set((APB_I2C5_BASE + 0x3c), 0x67F001F);
    writel_set((APB_I2C6_BASE + 0x3c), 0x67F001F);
    writel_set((APB_I2C7_BASE + 0x3c), 0x67F001F);
    writel_set((APB_I2C8_BASE + 0x3c), 0x67F001F);
}

static void uart_err_irq_enable(void)
{
    /* enable sem irq for uart */
    writel_set((APB_UART1_BASE + 0x3c), 0x67F1F1F);
    writel_set((APB_UART2_BASE + 0x3c), 0x67F1F1F);
    writel_set((APB_UART3_BASE + 0x3c), 0x67F1F1F);
    writel_set((APB_UART4_BASE + 0x3c), 0x67F1F1F);
    writel_set((APB_UART5_BASE + 0x3c), 0x67F1F1F);
    writel_set((APB_UART6_BASE + 0x3c), 0x67F1F1F);
    writel_set((APB_UART7_BASE + 0x3c), 0x67F1F1F);
    writel_set((APB_UART8_BASE + 0x3c), 0x67F1F1F);
    writel_set((APB_UART9_BASE + 0x3c), 0x67F1F1F);
    writel_set((APB_UART10_BASE + 0x3c), 0x67F1F1F);
    writel_set((APB_UART11_BASE + 0x3c), 0x67F1F1F);
    writel_set((APB_UART12_BASE + 0x3c), 0x67F1F1F);
    writel_set((APB_UART13_BASE + 0x3c), 0x67F1F1F);
    writel_set((APB_UART14_BASE + 0x3c), 0x67F1F1F);
    writel_set((APB_UART15_BASE + 0x3c), 0x67F1F1F);
    writel_set((APB_UART16_BASE + 0x3c), 0x67F1F1F);
}

static void vic_err_irq_enable(void)
{
    /* enable sem irq for vic */
    writel(0xFFFFFFFF, (VIC1_BASE + 0x1f00));
//   writel(0xFFFFFFFF, (VIC3PORTA_BASE + 0x1f00));
//   writel(0xFFFFFFFF, (VIC3PORTB_BASE + 0x1f00));
//   writel(0xFFFFFFFF, (VIC2PORTA_BASE + 0x1f00));
//   writel(0xFFFFFFFF, (VIC2PORTB_BASE + 0x1f00));
}

static void pmu_err_irq_enable(void)
{
    /* enable sem irq for pmu */
    writel_set((APB_PMU_CORE_BASE + 0x8008), 0xF);
    writel_set((APB_PMU_CORE_BASE + 0x6c), 0x11F0000);
    writel_set((APB_PMU_CORE_BASE + 0x68), 0x1FFF0000);
}

static void ethnet_err_irq_enable(void)
{
    /* enable sem irq for ethnet1 */
    writel_set((APB_ENET1_BASE + 0x2004), 0x3f);
    writel_set((APB_ENET1_BASE + 0x2008), 0x3f);
    writel_set((APB_ENET1_BASE + 0x2024), 0x3);
    writel_set((APB_ENET1_BASE + 0x2028), 0x3);
    writel_set((APB_ENET1_BASE + 0x2034), 0x3F3F3F3F);
    writel_set((APB_ENET1_BASE + 0x2038), 0x3F3F3F3F);
    /* enable sem irq for ethnet2 */
    writel_set((APB_ENET2_BASE + 0x2004), 0x3f);
    writel_set((APB_ENET2_BASE + 0x2008), 0x3f);
    writel_set((APB_ENET2_BASE + 0x2024), 0x3);
    writel_set((APB_ENET2_BASE + 0x2028), 0x3);
    writel_set((APB_ENET2_BASE + 0x2034), 0x3F3F3F3F);
    writel_set((APB_ENET2_BASE + 0x2038), 0x3F3F3F3F);
}

static void spi_err_irq_enable(void)
{
    writel_set(APB_SPI1_BASE + 0x44, 1);
    writel_set(APB_SPI1_BASE + 0x4C, 0x03FFF);
    writel_set(APB_SPI1_BASE + 0x54, 7);
    writel_set(APB_SPI2_BASE + 0x44, 1);
    writel_set(APB_SPI2_BASE + 0x4C, 0x03FFF);
    writel_set(APB_SPI2_BASE + 0x54, 7);
    writel_set(APB_SPI3_BASE + 0x44, 1);
    writel_set(APB_SPI3_BASE + 0x4C, 0x03FFF);
    writel_set(APB_SPI3_BASE + 0x54, 7);
    writel_set(APB_SPI4_BASE + 0x44, 1);
    writel_set(APB_SPI4_BASE + 0x4C, 0x03FFF);
    writel_set(APB_SPI4_BASE + 0x54, 7);
}

static void misc_err_irq_enable(void)
{
    writel_set(APB_GPIO_SF_BASE + 0xf10, 0xFE);
    writel_set(APB_GPIO_SF_BASE + 0xf14, 1);
}

static void clear_known_errors(void)
{
    /* Clear scr error */
    writel(0xEA, APB_SCR_SF_BASE + 0x4600);

    /* Clear fs_24m error */
    RMWREG32(APB_FS_24M_BASE + 0x614, 16U, 1U, 1);
    RMWREG32(APB_FS_24M_BASE + 0x614, 16U, 1U, 0);
}

static bool sem_ignore_corr_int(sem_corr_int_input input)
{
#if 0
    static const sem_corr_int_input ignore_corr_int[] = {0};

    for (size_t i = 0; i < ARRAY_SIZE(ignore_corr_int); i++) {
        if (ignore_corr_int[i] == input)
            return true;
    }
#endif
    return false;
}

static bool sem_ignore_uncorr_int(sem_uncorr_int_input input)
{
    static const sem_uncorr_int_input ignore_uncorr_int[] = {
        SEM_FS_24M_UNC_IRQ_NUM,
        SEM_SCR_SF_SCR_UNC_IRQ_NUM,
        SEM_APB_XSPI1A_SYNCBRIDGE_DST_UNCERR_NUM,
    };

    for (size_t i = 0; i < ARRAY_SIZE(ignore_uncorr_int); i++) {
        if (ignore_uncorr_int[i] == input)
            return true;
    }

    return false;
}

#if CONFIG_SEM_ERROR_INTR
static int sem_irq_handler(uint32_t irq, void *arg)
{
    printf("sem_irq_handler\n");
    sem_monitor_dump();

    return 0;
}
#endif

static void sem_status_check_init_power_on(void)
{
    clear_known_errors();
    scr_err_irq_enable();

    ldo_dcdc_err_irq_enable();
    i2c_err_irq_enable();
    uart_err_irq_enable();
    vic_err_irq_enable();
    pmu_err_irq_enable();
    spi_err_irq_enable();
    misc_err_irq_enable();

#if CONFIG_SEM_ERROR_INTR
    sdrv_sem_lld_override_en(APB_SEM1_BASE, SEM_OUTPUT_CPU, true);
    sdrv_sem_lld_override_en(APB_SEM2_BASE, SEM_OUTPUT_CPU, true);

    for (uint32_t i = 0; i < SEM_IOMUXC_RTC_IOMUX_CTRL_COR_IRQ_NUM + 1; i++) {
        if (!sem_ignore_corr_int(i)) {
            sdrv_sem_lld_corr_interrupt_en(APB_SEM1_BASE, SEM_OUTPUT_CPU, i, true);
            sdrv_sem_lld_corr_interrupt_en(APB_SEM2_BASE, SEM_OUTPUT_CPU, i, true);
        }
    }

    for (uint32_t j = 0; j < SEM_IOMUXC_RTC_IOMUX_CTRL_UNC_IRQ_NUM + 1; j++) {
        if (!sem_ignore_uncorr_int(i)) {
            sdrv_sem_lld_uncorr_interrupt_en(APB_SEM1_BASE, SEM_OUTPUT_CPU, j, true);
            sdrv_sem_lld_uncorr_interrupt_en(APB_SEM2_BASE, SEM_OUTPUT_CPU, j, true);
        }
    }

    irq_attach(SEM1_O_SEM_CPU_INTR_NUM, sem_irq_handler, NULL);
    irq_enable(SEM1_O_SEM_CPU_INTR_NUM);
#endif
}

static void sem_monitor_thread(void *ptr)
{
    unsigned int period = *(unsigned int *)ptr;

    printf("sem_monitor_thread, period %d\n", period);

    for (;;) {
        osDelay(period);
        sem_monitor_dump();
    }
}

void sem_monitor_dump(void)
{
    /* Check sem input interrupt status and notify to Appl */
    for (uint32_t i = 0U; i < SEM_IOMUXC_RTC_IOMUX_CTRL_COR_IRQ_NUM + 1; i++) {
        if (!sem_ignore_corr_int(i)) {
            if (sdrv_sem_lld_corr_int_status(APB_SEM1_BASE, i) ||
                sdrv_sem_lld_corr_int_status(APB_SEM2_BASE, i)) {
                printf("WARNING1: SEM Corr error detected. Please report it:%d\n", i);
            }
        }
    }

    /* Check sem input interrupt status and notify to Appl */
    for (uint32_t j = 0U; j < SEM_IOMUXC_RTC_IOMUX_CTRL_UNC_IRQ_NUM + 1; j++) {
        if (!sem_ignore_uncorr_int(j)) {
            if (sdrv_sem_lld_uncorr_int_status(APB_SEM1_BASE, j) ||
                sdrv_sem_lld_uncorr_int_status(APB_SEM2_BASE, j)) {
                printf("WARNING1: SEM UnCorr error detected. Please report it:%d\n", j);
            }
        }
    }
}

void sem_monitor_earlyinit(void)
{
    sem_status_check_init_power_on();
}

void sem_monitor_init()
{
    sem_monitor_earlyinit();
    sem_monitor_dump();

#if !defined CONFIG_SEM_ERROR_INTR && CONFIG_SEM_ERROR_CHECK_PERIOD > 0
    /* Monitor SEM errors periodically. */
    static unsigned int monitor_period = CONFIG_SEM_ERROR_CHECK_PERIOD;

    osThreadAttr_t attr = {
        .name = "sem_monitor",
        .priority = osPriorityNormal
    };
    osThreadNew(sem_monitor_thread, &monitor_period, &attr);
#endif
}
