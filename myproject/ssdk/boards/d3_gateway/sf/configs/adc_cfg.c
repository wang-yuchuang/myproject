/**
 * @file adc_cfg.c
 * @brief adc board support source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifdef CONFIG_ADC

#include <reg.h>
#include <board.h>


void board_adc_init(void)
{
    /* set MCP to all 1 */
    writel(0xFFFF, DEVICE_BASE(ADC1) + 0x244);
    writel(0xFFFF, DEVICE_BASE(ADC2) + 0x244);
    writel(0xFFFF, DEVICE_BASE(ADC3) + 0x244);
}
#endif

