/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
#ifndef BOARDS_SCR_CFG_H
#define BOARDS_SCR_CFG_H

#ifdef CONFIG_SCR

#include <driver/scr.h>

/* exported scr devices */
extern scr_dev_t g_scr_safety;
extern scr_dev_t g_scr_sec;
extern scr_dev_t g_scr_hpi;

void board_scr_init();
#endif

#endif /* BOARDS_SCR_CFG_H */
