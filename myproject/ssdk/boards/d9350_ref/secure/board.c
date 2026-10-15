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

#ifdef CONFIG_BOARD_REMAP
#include "remap.h"
#endif

#ifdef CONFIG_RESETGEN
#include "rstgen_cfg.h"
#endif

#ifdef CONFIG_PINCTRL
#include "pinmux_cfg.h"
#endif

#ifdef CONFIG_KUNLUN_MBOX

#include "mbox_cfg.h"
#endif

#ifdef CONFIG_DISPSS
#include "disp_cfg.h"
#endif

#define GLB_MAC_BASE_ADDR 0xf0BC0000
#define  MAC_GLB_CTL  0
#define  RPC_BASE_ADDR_CKGEN_SOC  0xf6800000
#define  RPC_MAC_GLB_CTL_OFFSET  0x7fe000
#define      RPC_BASE_ADDR_SEC  0xf8800000
#define SDRV_REG_STATUS 6
#define SDRV_REG_STATUS_FIREWALL_EARLY_DONE (1<<2)
#define SDRV_GENERAL_REG(n) (APB_RSTGEN_SEC_BASE + (uint32_t)((0x50 + (n)*4) << 10))

static inline void sdrv_general_reg_set(uint32_t reg, uint32_t value,
                                        uint32_t mask)
{
    uint32_t tmp_value;
    tmp_value = readl(SDRV_GENERAL_REG(reg)) & ~mask;
    tmp_value |= value & mask;
    writel(tmp_value, SDRV_GENERAL_REG(reg));
}

static void firewall_disable(void)
{
    uint32_t value = readl(GLB_MAC_BASE_ADDR + MAC_GLB_CTL);
    value = 0x10;
    writel(value, GLB_MAC_BASE_ADDR + MAC_GLB_CTL);
    writel(value, RPC_BASE_ADDR_CKGEN_SOC + RPC_MAC_GLB_CTL_OFFSET + MAC_GLB_CTL);
    writel(value, RPC_BASE_ADDR_SEC + RPC_MAC_GLB_CTL_OFFSET + MAC_GLB_CTL);

#ifdef ENABLE_SAF_FIREWALL
    writel(value, RPC_BASE_ADDR_SAF + RPC_MAC_GLB_CTL_OFFSET + MAC_GLB_CTL);
#endif
/* mp core */
    sdrv_general_reg_set(SDRV_REG_STATUS,
                            SDRV_REG_STATUS_FIREWALL_EARLY_DONE,
                            SDRV_REG_STATUS_FIREWALL_EARLY_DONE);
}
void board_early_init(void)
{

#ifdef CONFIG_RESETGEN
    board_reset_init();
#endif

#ifdef CONFIG_BOARD_REMAP
    board_remap();
#endif

#ifdef CONFIG_CLK
    board_clk_init();
#endif

#ifdef CONFIG_UART
    board_uart_early_init();
#endif

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

void board_init(void)
{
#ifdef CONFIG_UART
    /*
     * because secure core and ap2 core use the same uart,
     * after seure core had started should disable secure core uart,
     * in case to affect the use of ap2.
     */
    // board_uart_init();
#endif

#ifdef CONFIG_TIMER
    board_timer_init();
#endif
#ifdef CONFIG_KUNLUN_MBOX
    board_mbox_init();
#endif

#ifdef CONFIG_DISPSS
    board_disp_init();
#endif
}
