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

#include <driver/pwm.h>

/* exported pwm devices */
extern pwm_dev_t *g_pwm_1;
extern pwm_dev_t *g_pwm_2;
extern pwm_dev_t *g_pwm_3;
extern pwm_dev_t *g_pwm_4;

extern pwm_dev_t *g_pwm2_1;
extern pwm_dev_t *g_pwm2_2;
extern pwm_dev_t *g_pwm2_3;
extern pwm_dev_t *g_pwm2_4;

extern pwm_dev_t *g_pwm3_1;
extern pwm_dev_t *g_pwm3_2;
extern pwm_dev_t *g_pwm3_3;
extern pwm_dev_t *g_pwm3_4;

extern pwm_dev_t *g_pwm4_1;
extern pwm_dev_t *g_pwm4_2;
extern pwm_dev_t *g_pwm4_3;
extern pwm_dev_t *g_pwm4_4;

/* uart pwm functions */
extern void board_pwm_init(void);

#endif

#endif /* BOARDS_PWM_CFG_H */
