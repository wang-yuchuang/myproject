/*
 * poweroff.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Revision History:
 * -----------------
 */
#include <reg.h>
#include <board.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <cmsis_os2.h>
#include "CLI_type.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr)    (sizeof(arr) / sizeof((arr)[0]))
#endif

#ifndef udelay
#define udelay(x) spin(x)
#endif

typedef struct reg_desc {
	int reg;
	int val;
	int delay; /*us*/
} reg_desc_t;

typedef struct power_off {
	const char *name;
	reg_desc_t *rval;
	int rsize;
} power_off_t;

static reg_desc_t ddr_regs[] = {
	{0xf845d000, 0x2, 0},
	{0xf845e000, 0x2, 0},
	{0xf845f000, 0x2, 0},
	{0xf8460000, 0x2, 0}
};

static reg_desc_t cpu1_cluster_regs[] = {
	{0xf611b000, 0x2, 0},
	{0xf6136000, 0x2, 0},
	{0xf6137000, 0x2, 0},
	{0xf6138000, 0x2, 0},
	{0xf6139000, 0x2, 0},
	{0xf613a000, 0x2, 0},
	{0xf613b000, 0x2, 0},
	{0xf846d000, 0x2, 100}
};

static reg_desc_t cpu2_cluster_regs[] = {
	{0xf611c000, 0x2, 0},
	{0xf611d000, 0x2, 0},
	{0xf846e000, 0x2, 100}
};

static reg_desc_t gpu2_regs[] = {
	{0xf6120000, 0x2, 0},
	{0xf6121000, 0x2, 0},
	{0xf8492000, 0x2, 100},
	{0xf8493000, 0x2, 0}
};

static reg_desc_t ai_regs[] = {
	{0xf6127000, 0x2, 0},
	{0xf6128000, 0x2, 0},
	{0xf6129000, 0x2, 0},
	{0xf612a000, 0x2, 0},
	{0xf612b000, 0x2, 0},
	{0xf8452000, 0x2, 100}
};

static reg_desc_t vpu_regs[] = {
	{0xf6122000, 0x2, 0},
	{0xf6124000, 0x2, 0},
	{0xf6123000, 0x2, 0},
	{0xf6125000, 0x2, 0},
	{0xf8467000, 0x2, 100},
	{0xf8468000, 0x2, 0},
	{0xf8469000, 0x2, 0}
};

static reg_desc_t disp_regs[] = {
	{0xf6300000, 0x2, 0},
	{0xf630c000, 0x2, 0},
	{0xf630d000, 0x2, 0},
	{0xf630e000, 0x2, 0},
	{0xf630f000, 0x2, 0},
	{0xf847d000, 0x2, 100},
	{0xf847e000, 0x2, 0},
	{0xf847f000, 0x2, 0},
	{0xf8480000, 0x2, 0},
	{0xf6310000, 0x2, 100},
	{0xf8481000, 0x2, 100},
	{0xf6311000, 0x2, 100},
	{0xf6312000, 0x2, 0},
	{0xf6313000, 0x2, 0},
	{0xf8482000, 0x2, 100},
	{0xf8483000, 0x2, 0},
	{0xf8484000, 0x2, 0},
	{0xf8485000, 0x2, 100},
	{0xf847b000, 0x2, 0},
	{0xf847c000, 0x2, 0},
	{0xf8489000, 0x2, 0},
	{0xf848a000, 0x2, 0},
	{0xf848b000, 0x2, 0},
	{0xf6319000, 0x2, 0},
	{0xf631d000, 0x2, 0},
	{0xf631a000, 0x2, 0},
	{0xf631e000, 0x2, 0},
	{0xf631b000, 0x2, 0},
	{0xf631f000, 0x2, 0},
	{0xf8478000, 0x2, 100},
	{0xf8479000, 0x2, 0},
	{0xf847a000, 0x2, 0},
	{0xf8486000, 0x2, 0},
	{0xf8487000, 0x2, 0},
	{0xf8488000, 0x2, 0}
};

static power_off_t list[] = {
	{
		.name= "ddr",
		.rval= ddr_regs,
		.rsize= ARRAY_SIZE(ddr_regs),
	},
	{
		.name= "cpu1",
		.rval= cpu1_cluster_regs,
		.rsize= ARRAY_SIZE(cpu1_cluster_regs),
	},
	{
		.name= "cpu2",
		.rval= cpu2_cluster_regs,
		.rsize= ARRAY_SIZE(cpu2_cluster_regs),
	},
	{
		.name= "gpu2",
		.rval= gpu2_regs,
		.rsize= ARRAY_SIZE(gpu2_regs),
	},
	{
		.name= "ai",
		.rval= ai_regs,
		.rsize= ARRAY_SIZE(ai_regs),
	},
	{
		.name= "vpu",
		.rval= vpu_regs,
		.rsize= ARRAY_SIZE(vpu_regs),
	},
	{
		.name= "disp",
		.rval= disp_regs,
		.rsize= ARRAY_SIZE(disp_regs),
	},
};

static power_off_t *power_off_find_by_name(const char *name)
{
	unsigned int idx;

	for (idx = 0; idx < ARRAY_SIZE(list); idx++) {
		if (strncmp(list[idx].name, name, strlen(list[idx].name)) == 0)
			return &list[idx];
	}

	return NULL;
}

int cmd_poweroff(int argc, char *argv[])
{
	int idx;
	int rid;
	power_off_t *pwr;

	if (argc < 1) {
		ssdk_printf(SSDK_CRIT, "Invalid param\npoweroff [ddr|cpu1|cpu2|gpu2|ai|vpu|disp]\n");
		return CLI_FALSE;
	}

	for (idx = 0; idx < argc; idx++) {
		pwr = power_off_find_by_name(argv[idx]);
		if (!pwr)
			return CLI_FALSE;

		ssdk_printf(SSDK_CRIT, "\n[%s] power down\n", argv[idx]);
		for (rid = 0; rid < pwr->rsize; rid++) {
			if (pwr->rval[rid].delay)
				udelay(pwr->rval[rid].delay);
			writel(pwr->rval[rid].val, pwr->rval[rid].reg);
			printf("reg: 0x%x, val: 0x%x\n", pwr->rval[rid].reg, pwr->rval[rid].val);
		}
	}

	return CLI_FALSE;
}

CLI_CMD("poweroff", "poweroff [ddr|cpu1|cpu2|gpu2|ai|vpu|disp]", cmd_poweroff);
