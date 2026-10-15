// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <debug.h>
#include <types.h>
#include <stdbool.h>
#include <compiler.h>
#include <reg.h>
#include <param.h>
#include <board.h>
#include <debug.h>
#include <driver/scr.h>
#include "scr_hw.h"
#include <lib/sd_boot_pin/sd_boot_pin.h>
#include "driver/kunlun/fuse/fuse.h"

#define ERROR(format, args...) ssdk_printf(SSDK_CRIT, \
                               "[ERROR]SD_BOOT_IMG:%s %d "format"", __func__, __LINE__,  ##args);
#define DBG(format, args...) ssdk_printf(SSDK_CRIT, \
                               "[DEBUG]SD_BOOT_IMG:%s %d "format"", __func__, __LINE__,  ##args);

struct boot_info g_bootinfo;

scr_dev_t dev_scr_saf = {
        .base = APB_SCR_SAF_BASE,
        .lock = SPIN_LOCK_INITIAL_VALUE,
        .reg_shift = 10,
        .reg_off = {
          [TYPE_RW] = 0,
          [TYPE_RO] = 0x100,
          [TYPE_L16] = 0x200,
          [TYPE_L31] = 0x400,
          [TYPE_R16W16] = 0x600,
        },
};

/* Check SEC_SCR.L16[49] */
static uint32_t boot_get_pin_scr_overwrite(struct boot_info *bootinfo)
{
	/* Check SEC_SCR.L16[49][8] first */
	uint32_t data = 0;

	data = readl(SEC_BT_PIN_OVERRIDE_REG);
	if (data & (1 << 8)) {
		DBG("pin from scr\n");
		/* Get pin from SEC_SCR.L16[49][3:0] */
		data &= 0xf;
		bootinfo->boot_ops |= BOOT_INFO_FROM_SCR_MASK;
		bootinfo->boot_pin = data;
		return data;
	}
	return PIN_ERROR;
}

/* Check SEC_SCR.L16[49] */
void boot_set_pin_scr_overwrite(uint32_t pin)
{
	/* Check SEC_SCR.L16[49][8] first */
	pin &= 0xf;
	pin |= (1 << 8);

	writel(pin, SEC_BT_PIN_OVERRIDE_REG);
}

/* Check BT_FUSE0 */
static uint32_t boot_get_pin_fuse(struct boot_info *bootinfo)
{
	uint32_t data;

	data = fuse_read(FUSE_BT_CGF_INDEX0);

	/* BT_FUSE0[25], 0 from external boot pin, 1 from fuse */
	if (data & (1 << 25)) {
		/* Get pin from BT_FUSE0[31:26] */
		data = (data & 0x3c000000) >> 26;
		bootinfo->boot_ops |= BOOT_INFO_FROM_FUSE_MASK;
		bootinfo->boot_pin = data;
		return data;
	}

	return PIN_ERROR;
}

/* Check GPIO_B[11:8] */
static uint32_t boot_get_pin_gpio(struct boot_info *bootinfo)
{
	uint32_t data;
	scr_signal_t signal = SCR_SEC__rstgen_saf_boot_mode_scr_3_0;

	data = scr_get(&dev_scr_saf, &signal);
	data &= 0xf;
	bootinfo->boot_ops |= BOOT_INFO_FROM_GPIO_MASK;
	bootinfo->boot_pin = data;
	printf("boot pin mode is %d\n", data);
	return data;
}

static void update_boot_info(struct boot_info *bootinfo)
{
	uint32_t data, pin;

	if (bootinfo->boot_ops & BOOT_INFO_BOOT_PIN_UPDATE_MASK)
		return;

	pin = boot_get_pin_scr_overwrite(bootinfo);

	if (pin == PIN_ERROR)
		pin = boot_get_pin_fuse(bootinfo);

	if (pin == PIN_ERROR)
		pin = boot_get_pin_gpio(bootinfo);

	data = fuse_read(FUSE_BT_CGF_INDEX0);

	/*  BT_FUSE0[24], 0 - Enable, 1 - Disable */
	if (data & 1 << 24)
		bootinfo->boot_ops |= BOOT_INFO_USB_PROVISION_DIS_MASK;

	/* BT_FUSE0[18] */
	if (data & 1 << 18)
		bootinfo->boot_ops |= BOOT_INFO_SAFETY_HANDOVER_DIS_MASK;

	/* BT_FUSE0[17], 1 - Disable */
	if (data & 1 << 17)
		bootinfo->boot_ops |= BOOT_INFO_PEER_LOAD_ON_SAF_FAILURE_DIS_MASK;

	bootinfo->boot_ops |= BOOT_INFO_BOOT_PIN_UPDATE_MASK;
}

uint32_t boot_get_pin(void)
{
	update_boot_info(&g_bootinfo);

	return g_bootinfo.boot_pin;
}
