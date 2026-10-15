/**
 * @file board.c
 * @brief board config source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <types.h>
#include <compiler.h>
#include "board.h"
#include "driver/gpio.h"
#include "driver/kunlun/gpio/sdrv_gpio.h"
#include <reg.h>

#ifdef CONFIG_IRQ
#include <driver/irq.h>
#endif

#ifdef CONFIG_ARCH_USE_MPU
#include "mpu_cfg.h"
#endif

#ifdef CONFIG_UART
#include "uart_cfg.h"
#endif

#ifdef CONFIG_CLK
#include "clock_cfg.h"
#endif

#ifdef CONFIG_TIMER
#include "timer_cfg.h"
#endif

#ifdef CONFIG_RTC
#include "rtc_cfg.h"
#endif

#ifdef CONFIG_BOARD_REMAP
#include "remap.h"
#endif

#ifdef CONFIG_RESETGEN
#include "rstgen_cfg.h"
#endif

#ifdef CONFIG_PINCTRL
#include "pinmux_cfg.h"
#endif

#ifdef CONFIG_DW_DMA
#include "dma_cfg.h"
#endif

#if defined(CONFIG_DW_DMA) || defined(CONFIG_DW_PCIE_DMA)
#include "dma_cfg.h"
#endif

#ifdef CONFIG_DWC_PCIE
#include "pcie_cfg.h"
#endif

#ifdef CONFIG_KUNLUN_MBOX
#include "mbox_cfg.h"
#endif

#ifdef CONFIG_SYS_DIAGNOSIS
#include "sys_diagnosis.h"
#endif
#ifdef CONFIG_ETH
#include "eth_cfg.h"
#endif

#ifdef CONFIG_SCR
#include "scr_cfg.h"
#endif /* end CONFIG_SCR */

#ifdef CONFIG_DISPSS
#include "disp_cfg.h"
#endif

#ifdef CONFIG_SPI
#include "spi_cfg.h"
#endif

#ifdef CONFIG_SDRV_KUNLUN_PMU
#include "pmu_cfg.h"
#endif /* end CONFIG_SDRV_KUNLUN_PMU */

#define CKGEN_SYS_CNT 3000000
#define APB_SYS_CNT_RW_BASE (0xF1410000)
#define TSGEN_BASE APB_SYS_CNT_RW_BASE
#define CNTFID0 0x20
#define CNTCR 0

#define GLB_MAC_BASE_ADDR 0xf0BC0000
#define  MAC_GLB_CTL  0
#define  RPC_BASE_ADDR_CKGEN_SOC  0xf6800000
#define  RPC_MAC_GLB_CTL_OFFSET  0x7fe000
#define      RPC_BASE_ADDR_SEC  0xf8800000

#define SDRV_REG_STATUS_ADDR  (APB_RSTGEN_SEC_BASE + ((0x50 + (6) * 4) << 10))

static void firewall_disable(void)
{
    uint32_t regval;
    uint32_t value = readl(GLB_MAC_BASE_ADDR + MAC_GLB_CTL);
    //value = reg_value(dom_per_en, value, MAC_GLB_CTL_DOM_PRO_EN_SHIFT, MAC_GLB_CTL_DOM_PRO_EN_MASK);
    value = 0x10;
    writel(value, GLB_MAC_BASE_ADDR + MAC_GLB_CTL);
    writel(value, RPC_BASE_ADDR_CKGEN_SOC + RPC_MAC_GLB_CTL_OFFSET + MAC_GLB_CTL);
    writel(value, RPC_BASE_ADDR_SEC + RPC_MAC_GLB_CTL_OFFSET + MAC_GLB_CTL);

#ifdef ENABLE_SAF_FIREWALL
    writel(value, RPC_BASE_ADDR_SAF + RPC_MAC_GLB_CTL_OFFSET + MAC_GLB_CTL);
#endif

    /* indicate the firewall is disabled through the BIT2*/
    regval = readl(SDRV_REG_STATUS_ADDR);
    regval |= (1 << 2);
    writel(regval, SDRV_REG_STATUS_ADDR);
}

/* HPI QOS config */
void init_qos_by_module(addr_t iobase, int offset,
                        uint32_t pri, uint32_t mode,
                        uint32_t bw, uint32_t satur, uint32_t ext)
{
    /* priority */
    writel(pri, iobase + offset + 0x8);
    /* mode */
    writel(mode, iobase + offset + 0xc);
    /* bw */
    writel(bw, iobase + offset + 0x10);
    /* saturation */
    writel(satur, iobase + offset + 0x14);
    /* ext */
    writel(ext, iobase + offset + 0x18);
}

void hpi_qos_init(void)
{
    addr_t iobase = 0;
#define p2v
    iobase = p2v(APB_GPV_DISP_BASE);
    /* G2D1 */
    init_qos_by_module(iobase, 0x180, 0x80000503, 0x00000003, 0x0000016c,
                       0x00000080, 0x0);
    /* G2D2 */
    init_qos_by_module(iobase, 0x200, 0x80000606, 0x00000003, 0x0000016c,
                       0x00000080, 0x0);

#if 1
    /* regulator mode */
    /* CSI1 */
    init_qos_by_module(iobase, 0, 0x80000503, 0x00000003, 0x00000111, 0x00000080,
                       0x0);
    /* CSI2 */
    init_qos_by_module(iobase, 0x80, 0x80000503, 0x00000003, 0x00000111, 0x00000080,
                       0x0);
    /* CSI3 */
    init_qos_by_module(iobase, 0x100, 0x80000503, 0x00000003, 0x00000111,
                       0x00000080, 0x0);
    /* DP1 */
    init_qos_by_module(iobase, 0x280, 0x80000704, 0x00000003, 0x000002d9,
                       0x00000200, 0x0);
    /* DC1 */
    init_qos_by_module(iobase, 0x300, 0x80000704, 0x00000003, 0x000002d9,
                       0x00000080, 0x0);
    /* DP2 */
    init_qos_by_module(iobase, 0x380, 0x80000704, 0x00000003, 0x000002d9,
                       0x00000200, 0x0);
    /* DC2 */
    init_qos_by_module(iobase, 0x400, 0x80000704, 0x00000003, 0x000002d9,
                       0x00000080, 0x0);
    /* DP3 */
    init_qos_by_module(iobase, 0x480, 0x80000704, 0x00000003, 0x000002d9,
                       0x00000200, 0x0);
    /* DC3 */
    init_qos_by_module(iobase, 0x500, 0x80000704, 0x00000003, 0x000002d9,
                       0x00000080, 0x0);
    /* DC4 */
    init_qos_by_module(iobase, 0x580, 0x80000704, 0x00000003, 0x0000016c,
                       0x00000080, 0x0);
    /* DC5 */
    init_qos_by_module(iobase, 0x600, 0x80000704, 0x00000003, 0x0000016c,
                       0x00000080, 0x0);
#else
    /* bypass mode */
    /* CSI1 */
    init_qos_by_module(iobase, 0, 0x80000503, 0x00000002, 0x0000016c, 0x00000080,
                       0x0);
    /* CSI2 */
    init_qos_by_module(iobase, 0x80, 0x80000503, 0x00000002, 0x0000005b, 0x00000080,
                       0x0);
    /* CSI3 */
    init_qos_by_module(iobase, 0x100, 0x80000503, 0x00000002, 0x0000005b,
                       0x00000080, 0x0);
    /* DP1 */
    init_qos_by_module(iobase, 0x280, 0x80000503, 0x00000002, 0x00000390,
                       0x00000200, 0x0);
    /* DC1 */
    init_qos_by_module(iobase, 0x300, 0x80000503, 0x00000002, 0x000000B6,
                       0x00000100, 0x0);
    /* DP2 */
    init_qos_by_module(iobase, 0x380, 0x80000503, 0x00000002, 0x00000390,
                       0x00000200, 0x0);
    /* DC2 */
    init_qos_by_module(iobase, 0x400, 0x80000503, 0x00000002, 0x000000b6,
                       0x00000100, 0x0);
    /* DP3 */
    init_qos_by_module(iobase, 0x480, 0x80000503, 0x00000002, 0x000001c8,
                       0x00000200, 0x0);
    /* DC3 */
    init_qos_by_module(iobase, 0x500, 0x80000503, 0x00000002, 0x0000005b,
                       0x00000100, 0x0);
    /* DC4 */
    init_qos_by_module(iobase, 0x580, 0x80000503, 0x00000002, 0x00000049,
                       0x00000100, 0x0);
    /* DC5 */
    init_qos_by_module(iobase, 0x600, 0x80000503, 0x00000002, 0x00000049,
                       0x00000100, 0x0);
#endif

#if 1
    iobase = p2v(APB_GPV_HIS_BASE);
    /* PCIE1 */
    init_qos_by_module(iobase, 0, 0x80000301, 0x00000003, 0x0000020a, 0x00000040,
                       0x0);
    /* PCIE2 */
    init_qos_by_module(iobase, 0x80, 0x80000402, 0x00000003, 0x0000020a, 0x00000040,
                       0x0);
    /* USB1 */
    init_qos_by_module(iobase, 0x100, 0x80000402, 0x00000003, 0x000001eb,
                       0x00000040, 0x0);
    /* USB2 */
    init_qos_by_module(iobase, 0x180, 0x80000402, 0x00000003, 0x000001eb,
                       0x00000040, 0x0);
#endif

    iobase = p2v(APB_GPV_VPU_BASE);
    /* vpu1.prim */
    init_qos_by_module(iobase, 0, 0x80000301, 0x00000003, 0x00000120, 0x00000040,
                       0x0);
    /* vpu1.2nd */
    init_qos_by_module(iobase, 0x80, 0x80000301, 0x00000003, 0x00000000, 0x00000040,
                       0x0);
    /* vpu1.proc */
    init_qos_by_module(iobase, 0x100, 0x80000301, 0x00000003, 0x00000030,
                       0x00000040, 0x0);
    /* vpu1.dma */
    init_qos_by_module(iobase, 0x180, 0x80000301, 0x00000003, 0x00000090,
                       0x00000040, 0x0);
    /* vpu1.sdma */
    init_qos_by_module(iobase, 0x200, 0x80000301, 0x00000003, 0x00000090,
                       0x00000040, 0x0);
    /* vpu2.prim */
    init_qos_by_module(iobase, 0x280, 0x80000301, 0x00000003, 0x00000090,
                       0x00000040, 0x0);
    /* vpu2.2nd */
    init_qos_by_module(iobase, 0x300, 0x80000301, 0x00000003, 0x00000000,
                       0x00000040, 0x0);
    /* MJPEG */
    init_qos_by_module(iobase, 0x380, 0x80000301, 0x00000003, 0x000000d8,
                       0x00000040, 0x0);

#if 0
    iobase = p2v(GPV_VSN_BASE);
    /* VDSP.prim */
    init_qos_by_module(iobase, 0x100, 0x80000301, 0x00000003, 0x0000004c,
                       0x00000040, 0x0);
    /* VDSP.dma */
    init_qos_by_module(iobase, 0x180, 0x80000301, 0x00000003, 0x00000180,
                       0x00000040, 0x0);
#endif

    iobase = p2v(APB_GPV_HPIA_BASE);
    /* CPU1.prim */
    init_qos_by_module(iobase, 0x2100, 0x80000403, 0x00000003, 0x00000000,
                       0x00000010, 0x0);
    /* CPU2.prim */
    init_qos_by_module(iobase, 0x2180, 0x80000403, 0x00000003, 0x00000000,
                       0x00000010, 0x0);
    /* CPU1.2nd */
    init_qos_by_module(iobase, 0x2200, 0x80000403, 0x00000003, 0x00000090,
                       0x00000010, 0x0);
    /* CPU2.2nd */
    init_qos_by_module(iobase, 0x2280, 0x80000403, 0x00000003, 0x00000090,
                       0x00000010, 0x0);
    /* TCU */
    init_qos_by_module(iobase, 0x2300, 0x80000503, 0x00000003, 0x0000001C,
                       0x00000008, 0x0);
    /* gpu1.a */
    init_qos_by_module(iobase, 0x2480, 0x80000301, 0x00000003, 0x00000120,
                       0x00000080, 0x0);
    /* gpu1.b */
    init_qos_by_module(iobase, 0x2500, 0x80000301, 0x00000003, 0x00000120,
                       0x00000080, 0x0);
    /* gpu2 */
    init_qos_by_module(iobase, 0x2580, 0x80000301, 0x00000003, 0x00000120,
                       0x00000080, 0x0);

    iobase = p2v(APB_GPV_SEC_M_BASE);
    /* CE2 */
    init_qos_by_module(iobase, 0x480, 0x80000301, 0x00000003, 0x000000f5,
                       0x00000020, 0x0);
    /* ENET2 */
    init_qos_by_module(iobase, 0x500, 0x80000402, 0x00000003, 0x0000004c,
                       0x00000010, 0x0);
    /* MSHC1 */
    init_qos_by_module(iobase, 0x580, 0x80000301, 0x00000003, 0x000000f5,
                       0x00000020, 0x0);
    /* MSHC2 */
    init_qos_by_module(iobase, 0x600, 0x80000301, 0x00000003, 0x0000007a,
                       0x00000020, 0x0);
    /* MSHC3 */
    init_qos_by_module(iobase, 0x680, 0x80000301, 0x00000003, 0x0000007a,
                       0x00000020, 0x0);
    /* MSHC4 */
    init_qos_by_module(iobase, 0x700, 0x80000301, 0x00000003, 0x0000003d,
                       0x00000020, 0x0);
    /* DMA2 */
    init_qos_by_module(iobase, 0x780, 0x80000301, 0x00000003, 0x0000003d,
                       0x00000008, 0x0);
    /* DMA3 */
    init_qos_by_module(iobase, 0x800, 0x80000301, 0x00000003, 0x0000003d,
                       0x00000008, 0x0);
    /* DMA4 */
    init_qos_by_module(iobase, 0x880, 0x80000301, 0x00000003, 0x0000003d,
                       0x00000008, 0x0);
    /* DMA5 */
    init_qos_by_module(iobase, 0x900, 0x80000301, 0x00000003, 0x0000003d,
                       0x00000008, 0x0);
    /* DMA6 */
    init_qos_by_module(iobase, 0x980, 0x80000301, 0x00000003, 0x0000003d,
                       0x00000008, 0x0);
    /* DMA7 */
    init_qos_by_module(iobase, 0xa00, 0x80000301, 0x00000003, 0x0000003d,
                       0x00000008, 0x0);
    /* DMA8 */
    init_qos_by_module(iobase, 0xa80, 0x80000301, 0x00000003, 0x0000003d,
                       0x00000008, 0x0);

    iobase = p2v(APB_GPV_SEC_BASE);
    /* cr5_sec */
    init_qos_by_module(iobase, 0x100, 0x80000503, 0x00000003, 0x00000099,
                       0x00000008, 0x0);

  //  if (0 != RES_GATING_EN_SEC_MP_PLAT) {
  //      /* cr5_mp */
  //      init_qos_by_module(iobase, 0x180, 0x80000503, 0x00000003, 0x00000099,
  //                         0x00000008, 0x0);
  //  }

    /* safety core resources */
#if 0
    iobase = p2v(APB_GPV_SAF_BASE);
    /* cr5_saf.prim */
    init_qos_by_module(iobase, 0x100, 0x80000503, 0x00000003, 0x00000000,
                       0x00000008, 0x0);
    /* cr5_saf.perp */
    init_qos_by_module(iobase, 0x180, 0x80000503, 0x00000003, 0x00000099,
                       0x00000002, 0x0);
    /* ce1 */
    init_qos_by_module(iobase, 0x200, 0x80000301, 0x00000003, 0x0000007a,
                       0x00000010, 0x0);
    /* dma1 */
    init_qos_by_module(iobase, 0x280, 0x80000301, 0x00000003, 0x0000003d,
                       0x00000008, 0x0);
    /* enet1 */
    init_qos_by_module(iobase, 0x300, 0x80000402, 0x00000003, 0x0000004c,
                       0x00000010, 0x0);
#endif
}

void board_early_init(void)
{
#ifdef CONFIG_SCR
    board_fsrefclk_config();
#endif

#ifdef CONFIG_UART
    board_uart_early_init();
#endif

    scr_init_eth();

#ifdef CONFIG_KUNLUN_RESETGEN
    board_reset_init();
#endif

#ifdef CONFIG_BOARD_REMAP
    board_remap();
#endif

#ifdef CONFIG_CLK
    board_clk_init();
#endif

    scr_init_i2s();

    /* save the frequency(CKGEN_SYS_CNT) to CNTFID0 register, we use it to
     * report the reader what the clock frequency of the global counter is
     */
 	writel(CKGEN_SYS_CNT, (TSGEN_BASE + CNTFID0));
    RMWREG32((TSGEN_BASE + CNTCR), 0, 1, 1);
	hpi_qos_init();

#ifdef CONFIG_ARCH_USE_MPU
    board_mpu_init();
#endif

#ifdef CONFIG_FIREWALL_DISABLE
	firewall_disable();
#endif

#ifdef CONFIG_IRQ
    irq_initialize(INT_BASE, IRQ_MAX_INTR_NUM);
#endif

#ifdef CONFIG_PINCTRL
    board_port_init();
#endif
}

/*
 * phy reset Sequence
 *
 * GPIO assignments (BMU05-A00):
 *   GPIO_D9  (IO73) = PHY1_RST
 *   GPIO_D8  (IO72) = PHY2_RST
 *
 * Sequence: After external watchdog fixed, phy reset sequence need under control by soc
 */

#define PIN_PHY1_RST       73   /* GPIO_D9, phy1 reset, DEFAULT AND ACTIVE :LOW(at least 10ms) */
#define PIN_PHY2_RST       72   /* GPIO_D8, phy2 reset, DEFAULT AND ACTIVE :LOW(at least 10ms)*/
extern gpio_dev_t g_gpio_ap1;
static void phy_reset_sequence(void)
{
    gpio_dev_t *g = &g_gpio_ap1;

    printf("Starting phy reset sequence...\n");

    /* Step 1: PHY reset pin default low, no need to pull low pins, just Wait 15ms */
    udelay(15000);

    /* Step 2: pull high phy reset pins*/
    gpio_set(g, PIN_PHY1_RST, true);
    gpio_set(g, PIN_PHY2_RST, true);

    printf("phy reset sequence complete\n");
}


void board_init(void)
{
#ifdef CONFIG_SCR
    board_scr_init();
#endif
#ifdef CONFIG_UART
    board_uart_init();
#endif

#ifdef CONFIG_TIMER
    board_timer_init();
#endif

    /* ssdk default pull low phy reset pins, so need pull high after 15ms. */
    phy_reset_sequence();

#ifdef CONFIG_RTC
    board_rtc_init();
#endif
#if defined(CONFIG_DW_DMA) || defined(CONFIG_DW_PCIE_DMA)
    board_dma_init();
#endif

#ifdef CONFIG_SDRV_KUNLUN_PMU
    board_pmu_init();
#endif /* end CONFIG_SDRV_KUNLUN_PMU */

#ifdef CONFIG_DWC_PCIE
    board_pcie_init();
#endif

#ifdef CONFIG_KUNLUN_MBOX
    board_mbox_init();
#endif

#ifdef CONFIG_SYS_DIAGNOSIS
    sysd_start(SEM1);
#endif

#ifdef CONFIG_DW_DMA
    board_dma_init();
#endif

#ifdef CONFIG_DISPSS
    board_disp_init();
#endif

#ifdef CONFIG_ETH
    board_eth_init();
#endif

#ifdef CONFIG_SPI
    board_spi_init();
#endif
}
