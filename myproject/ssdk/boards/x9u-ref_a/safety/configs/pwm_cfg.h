//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef BOARDS_PWM_CFG_H
#define BOARDS_PWM_CFG_H

#ifdef CONFIG_PWM

#include <driver/kunlun/pwm/pwm_drv.h>

/* exported pwm devices */
extern pwm_instance_t g_pwm3, g_pwm4, g_pwm5;

/* uart pwm functions */
extern void board_pwm_init(void);

#endif

#endif /* BOARDS_PWM_CFG_H */
