/*
 * poc-max2008x.h
 *
 * Semidrive platform csi header file
 *
 * Copyright (C) 2021, Semidrive  Communications Inc.
 *
 * This file is licensed under a dual GPLv2 or X11 license.
 */

#ifndef POC_MAX2008X_H
#define POC_MAX2008X_H

#include "exvdev.h"
struct poc_cfg {
	u8 poc_i2c_addr;
	u8 gpioext_addr;
	unsigned int poc_gpio;
	char *gpioext_name;
};

int poc_power(i2c_adap_dev_t *client, struct poc_cfg *cfg, int chan, int enable);

#endif
