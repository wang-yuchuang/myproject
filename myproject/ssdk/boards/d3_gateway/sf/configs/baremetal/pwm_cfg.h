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
extern pwm_dev_t *mc_chnl1;
extern pwm_dev_t *mc_chnl2;
extern pwm_dev_t *mc_chnl3;
extern pwm_dev_t *mc_chnl4;

/* uart pwm functions */
extern void board_pwm_init(void);

#endif

#endif /* BOARDS_PWM_CFG_H */
