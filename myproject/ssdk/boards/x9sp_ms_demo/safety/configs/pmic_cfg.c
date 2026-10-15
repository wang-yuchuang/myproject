/**
 * @file pmic_cfg.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: pmic config
 *
 * Revision History:
 * -----------------
*/
#ifdef CONFIG_PMIC
#include <debug.h>
#include <arch/register.h>
#include <regs_base.h>
#include <types.h>
#include "pmic_cfg.h"
#include "i2c_cfg.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#define PMIC1_DEV_ID    0x60
#define PMIC2_DEV_ID    0x61

struct power_item_t *g_power_configs = NULL;
int g_power_item_count = 0;

static struct pmic_dev_t lp87565 = {0};
static struct pmic_dev_t lp87523 = {0};
struct power_item_t power_configs[] = {
    {VDD_AP, &lp87565, LP875XX_BUCK0_IDX},
    {VDD_CPU, &lp87565, LP875XX_BUCK2_IDX},
    {VDD_GPU, &lp87523, LP875XX_BUCK0_IDX},
    {VDDQ_1V1, &lp87523, LP875XX_BUCK2_IDX},
    {VDDIO_1V8, &lp87523, LP875XX_BUCK3_IDX},
};

bool power_add_config(struct power_item_t *power_config, int count)
{
    g_power_configs = power_config;
    g_power_item_count = count;
    return true;
}

int32_t board_pmic_init()
{
    pmic_dev_add_lp875xx(&lp87565, &g_i2c3_adap_dev, PMIC1_DEV_ID, 4);
    pmic_dev_add_lp875xx(&lp87523, &g_i2c3_adap_dev, PMIC2_DEV_ID, 4);
    power_add_config(power_configs, ARRAY_SIZE(power_configs));

    return 0;
}
#endif
/* end of file pmic_cfg.c */