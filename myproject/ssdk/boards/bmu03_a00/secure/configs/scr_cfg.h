
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef BOARDS_SCR_CFG_H
#define BOARDS_SCR_CFG_H

#ifdef CONFIG_SCR

#include <driver/scr.h>

/* exported scr devices */
extern scr_dev_t g_scr_safety;
extern scr_dev_t g_scr_sec;
extern scr_dev_t g_scr_hpi;

void board_scr_init();
void board_fsrefclk_config();
void scr_init_eth();
void scr_init_i2s();

#endif

#endif /* BOARDS_SCR_CFG_H */
