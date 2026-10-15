/**
 * @file pmic_cfg.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: pmic config header file
 *
 * Revision History:
 * -----------------
 */
#ifdef CONFIG_PMIC
#ifndef INCLUDE_PMIC_CFG_H
#define INCLUDE_PMIC_CFG_H

#include "lp875xx.h"

extern struct power_item_t *g_power_configs;
extern int g_power_item_count;

int32_t board_pmic_init();

#endif /* INCLUDE_PMIC_CFG_H */
#endif
/* end of header file pmic_cfg.h */