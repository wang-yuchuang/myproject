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

#ifdef CONFIG_MBOX
#include "mbox_cfg.h"
#endif

#ifdef CONFIG_DW_DMA
#include "dma_cfg.h"
#endif

#ifdef CONFIG_DWC_PCIE
#include "pcie_cfg.h"
#endif

#ifdef CONFIG_ARCH_USE_MMU
#include "arch/arm64/mmu.h"


static struct mm_region d9_ref_mem_map[] = {
	{
		/* RAM */
		.virt = CONFIG_MEM_BASE,
		.phys = CONFIG_MEM_BASE,
		.size = CONFIG_MEM_SIZE,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* IO SPACE(OSPI AHB_MEM) */
		.virt = 0x4000000UL,
		.phys = 0x4000000UL,
		.size = 0x8000000UL,	/* 128MB */
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* IO SPACE */
		.virt = 0x30000000UL,
		.phys = 0x30000000UL,
		.size = 0x10000000UL,	/* 256MB */
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = d9_ref_mem_map;
#endif

#ifdef CONFIG_I2C
#include "i2c_cfg.h"
#endif

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

#ifdef CONFIG_IRQ
    irq_initialize(INT_BASE, IRQ_MAX_INTR_NUM);
#endif

#ifdef CONFIG_PINCTRL
    board_port_init();
#endif

#ifdef CONFIG_I2C
    board_i2c_early_init();
#endif
}

void board_init(void)
{
#ifdef CONFIG_UART
    board_uart_init();
#endif

#ifdef CONFIG_TIMER
    board_timer_init();
#endif

#ifdef CONFIG_MBOX
    board_mbox_init();
#endif

#ifdef CONFIG_DW_DMA
    board_dma_init();
#endif

#ifdef CONFIG_DWC_PCIE
    board_pcie_init();
#endif

#ifdef CONFIG_I2C
    board_i2c_init();
#endif
}
