
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef BOARDS_TIMER_CFG_H
#define BOARDS_TIMER_CFG_H

#ifdef CONFIG_TIMER

#include <driver/timer.h>

/* exported timer devices */
extern timer_dev_t *g_kernel_timer;

/* timer init functions */
extern void board_timer_init(void);

#endif

#endif /* BOARDS_TIMER_CFG_H */
