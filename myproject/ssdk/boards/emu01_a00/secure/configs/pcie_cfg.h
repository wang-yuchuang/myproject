/*
 * pcie_cfg.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Board level pcie configuration.
 *
 * Revision History:
 * -----------------
 */

#ifndef _PCIE_CFG_H_
#define _PCIE_CFG_H_

#ifdef CONFIG_DWC_PCIE

#include "driver/kunlun/pcie/pcie.h"

extern pcie_dev_t g_pcie1_dev;
extern pcie_dev_t g_pcie2_dev;

void board_pcie_init(void);

#endif

#endif
